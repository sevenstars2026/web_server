#include "Connection.h"
#include "Epoll.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <map>

int main() {
    // ========== 第一步：创建 socket ==========
    int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        std::cerr << "socket failed\n";
        return 1;
    }
    std::cout << "[step1] socket created, fd=" << listenFd << "\n";

    // ========== 第二步：绑定到端口 ==========
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (::bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind failed\n";
        return 1;
    }
    std::cout << "[step1] bind to 0.0.0.0:8080\n";

    // ========== 第三步：进入监听状态 ==========
    if (::listen(listenFd, SOMAXCONN) < 0) {
        std::cerr << "listen failed\n";
        return 1;
    }
    std::cout << "[step1] listening for connections...\n";

    // ========== 第四步：设置 listen_fd 为非阻塞 ==========
    int flags = ::fcntl(listenFd, F_GETFL);
    ::fcntl(listenFd, F_SETFL, flags | O_NONBLOCK);

    // ========== 第五步：创建 epoll 并添加 listen_fd ==========
    web::Epoll epoll;
    epoll.addFd(listenFd, EPOLLIN);
    std::cout << "[epoll] added listen_fd\n";

    // ========== 第六步：管理所有连接 ==========
    std::map<int, web::Connection> connections;  // fd -> Connection
    epoll_event events[64];

    std::cout << "[epoll] entering main loop...\n";

    // ========== 第七步：事件循环 ==========
    while (true) {
        int numEvents = epoll.wait(events, 64, -1);

        for (int i = 0; i < numEvents; ++i) {
            int fd = events[i].data.fd;

            // ===== 新连接到达 =====
            if (fd == listenFd) {
                sockaddr_in peer{};
                socklen_t peerLen = sizeof(peer);

                int connfd = ::accept(listenFd, reinterpret_cast<sockaddr*>(&peer), &peerLen);
                if (connfd < 0) {
                    std::cerr << "accept failed\n";
                    continue;
                }

                char clientIp[INET_ADDRSTRLEN];
                ::inet_ntop(AF_INET, &peer.sin_addr, clientIp, INET_ADDRSTRLEN);
                uint16_t clientPort = ntohs(peer.sin_port);

                std::cout << "[accept] new connection from " << clientIp << ":" << clientPort << "\n";

                // 设置为非阻塞
                flags = ::fcntl(connfd, F_GETFL);
                ::fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

                // 创建连接对象并加入 map
                web::Connection conn;
                conn.fd = connfd;
                std::strcpy(conn.clientIp, clientIp);
                conn.clientPort = clientPort;
                connections[connfd] = std::move(conn);

                // 添加到 epoll，监听可读事件
                epoll.addFd(connfd, EPOLLIN);

            }
            // ===== 现有连接有可读事件 =====
            else if (events[i].events & EPOLLIN) {
                auto it = connections.find(fd);
                if (it == connections.end()) continue;

                web::Connection& conn = it->second;
                if (!web::handleRead(conn)) {
                    // 连接需要关闭
                    epoll.removeFd(fd);
                    ::close(fd);
                    connections.erase(it);
                    continue;
                }

                // 如果有数据要写，改为监听可写事件
                if (!conn.writeBuffer.empty()) {
                    epoll.modFd(fd, EPOLLOUT | EPOLLIN);
                }
            }
            // ===== 现有连接有可写事件 =====
            else if (events[i].events & EPOLLOUT) {
                auto it = connections.find(fd);
                if (it == connections.end()) continue;

                web::Connection& conn = it->second;
                if (!web::handleWrite(conn)) {
                    // 连接需要关闭
                    epoll.removeFd(fd);
                    ::close(fd);
                    connections.erase(it);
                    continue;
                }

                // 如果写缓冲区为空，回到只监听可读
                if (conn.writeBuffer.empty()) {
                    epoll.modFd(fd, EPOLLIN);
                }
            }
        }
    }

    ::close(listenFd);
    return 0;
}
