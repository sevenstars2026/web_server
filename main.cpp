#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    // ========== 第一步：创建 socket ==========
    // AF_INET = IPv4
    // SOCK_STREAM = TCP（有序、可靠、面向连接）
    // 返回的 listenFd 就是这个通信端点的"句柄"，像文件描述符一样
    int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) {
        std::cerr << "socket failed\n";
        return 1;
    }
    std::cout << "[step0] socket created, fd=" << listenFd << "\n";

    // ========== 第二步：绑定到端口 ==========
    // sockaddr_in 是 IPv4 地址结构体
    sockaddr_in addr{};
    addr.sin_family = AF_INET;              // IPv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // 监听所有网卡（0.0.0.0）
    addr.sin_port = htons(8080);            // 端口 8080（htons 把主机字节序转成网络字节序）

    if (::bind(listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind failed\n";
        return 1;
    }
    std::cout << "[step0] bind to 0.0.0.0:8080\n";

    // ========== 第三步：进入监听状态 ==========
    // SOMAXCONN = 内核建议的最大待连接队列长度
    // 这个调用后，操作系统开始接受来自客户端的连接请求
    if (::listen(listenFd, SOMAXCONN) < 0) {
        std::cerr << "listen failed\n";
        return 1;
    }
    std::cout << "[step0] listening for connections...\n";

    // ========== 第四步：等待客户端连接（阻塞！） ==========
    // accept 会阻塞，直到有客户端连接过来
    // 返回的 connfd 是这个客户端连接的文件描述符
    // 如果想处理多个连接，这里就是瓶颈：只能一个一个处理
    sockaddr_in peer{};
    socklen_t peerLen = sizeof(peer);
    int connfd = ::accept(listenFd, reinterpret_cast<sockaddr*>(&peer), &peerLen);
    if (connfd < 0) {
        std::cerr << "accept failed\n";
        return 1;
    }

    // 把网络字节序的 IP 地址转回可读的字符串
    char clientIp[INET_ADDRSTRLEN];
    ::inet_ntop(AF_INET, &peer.sin_addr, clientIp, INET_ADDRSTRLEN);
    std::cout << "[step0] accepted connection from " << clientIp << ":"
              << ntohs(peer.sin_port) << "\n";

    // ========== 第五步：收发数据 ==========
    // read: 从 connfd 读数据（阻塞！）
    // write: 向 connfd 写数据
    char buf[1024];
    while (true) {
        // 阻塞读：等待客户端发来数据
        const ssize_t n = ::read(connfd, buf, sizeof(buf));
        if (n < 0) {
            std::cerr << "read error\n";
            break;
        }
        if (n == 0) {
            // 客户端关闭连接
            std::cout << "[step0] client closed\n";
            break;
        }

        // 收到 n 个字节
        std::cout << "[step0] received " << n << " bytes: ";
        for (int i = 0; i < n; ++i) {
            std::cout << buf[i];
        }
        std::cout << "\n";

        // 原样回复给客户端
        if (::write(connfd, buf, n) < 0) {
            std::cerr << "write error\n";
            break;
        }
        std::cout << "[step0] sent " << n << " bytes back\n";
    }

    // ========== 第六步：关闭连接 ==========
    ::close(connfd);
    ::close(listenFd);
    std::cout << "[step0] connection closed\n";

    return 0;
}
