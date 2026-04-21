#include "Connection.h"
#include "ThreadPool.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

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

    // ========== 第四步：创建线程池 ==========
    web::ThreadPool pool(8);  // 创建 8 个工作线程
    std::cout << "[step2] thread pool created\n";

    // ========== 第五步：循环等待连接 ==========
    while (true) {
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

        std::cout << "[step2] accepted connection from " << clientIp << ":" << clientPort << "\n";

        // 提交任务给线程池
        pool.submit([connfd, clientIp, clientPort]() {
            web::handleConnection(connfd, clientIp, clientPort);
        });

        std::cout << "[step2] main thread back to accept...\n";
    }

    ::close(listenFd);
    return 0;
}
