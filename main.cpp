#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

// ========== 新增：处理连接的函数 ==========
// 这个函数会在独立的线程里运行
// connfd: 这个连接的文件描述符
// clientIp: 客户端 IP
// clientPort: 客户端端口
void handleConnection(int connfd, const char* clientIp, uint16_t clientPort) {
    std::cout << "[step1] thread " << std::this_thread::get_id() << " handling connection from " 
              << clientIp << ":" << clientPort << "\n";

    char buf[1024];
    while (true) {
        // 阻塞读：等待这个客户端发来数据
        const ssize_t n = ::read(connfd, buf, sizeof(buf));
        if (n < 0) {
            std::cerr << "[step1] read error\n";
            break;
        }
        if (n == 0) {
            // 客户端关闭连接
            std::cout << "[step1] client " << clientIp << ":" << clientPort << " closed\n";
            break;
        }

        // 收到 n 个字节
        std::cout << "[step1] received " << n << " bytes from " << clientIp << ": ";
        for (int i = 0; i < n; ++i) {
            std::cout << buf[i];
        }
        std::cout << "\n";

        // 原样回复给客户端
        if (::write(connfd, buf, n) < 0) {
            std::cerr << "[step1] write error\n";
            break;
        }
        std::cout << "[step1] sent " << n << " bytes back to " << clientIp << "\n";
    }

    // 关闭这个连接
    ::close(connfd);
    std::cout << "[step1] connection closed, thread " << std::this_thread::get_id() << " exit\n";
}

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

    // ========== 第四步：循环等待连接（关键改动！） ==========
    // 之前是 accept 一次就退出
    // 现在是 while(true) 循环，每次 accept 一个连接
    while (true) {
        sockaddr_in peer{};
        socklen_t peerLen = sizeof(peer);
        
        // 主线程在这里阻塞等待连接
        int connfd = ::accept(listenFd, reinterpret_cast<sockaddr*>(&peer), &peerLen);
        if (connfd < 0) {
            std::cerr << "accept failed\n";
            continue;  // 继续等待下一个连接
        }

        // 把网络字节序的 IP 转回可读格式
        char clientIp[INET_ADDRSTRLEN];
        ::inet_ntop(AF_INET, &peer.sin_addr, clientIp, INET_ADDRSTRLEN);
        uint16_t clientPort = ntohs(peer.sin_port);

        std::cout << "[step1] accepted connection from " << clientIp << ":" << clientPort << "\n";

        // ========== 新增：创建新线程处理这个连接 ==========
        // std::thread(...).detach() 的含义：
        //   - std::thread: 创建一个新线程
        //   - 括号里的 lambda: 新线程要执行的代码
        //   - .detach(): 将线程与主线程分离，让它独立运行
        //               主线程不需要等待它完成，继续去 accept 下一个连接
        std::thread([connfd, clientIp, clientPort]() {
            handleConnection(connfd, clientIp, clientPort);
        }).detach();

        // 主线程立即返回这里，继续循环等待下一个连接
        std::cout << "[step1] main thread back to accept...\n";
    }

    // ========== 清理资源 ==========
    // 注意：这段代码永远不会执行（while(true) 无限循环）
    // 在实际程序中，你可以通过按 Ctrl+C 来中断程序
    ::close(listenFd);
    return 0;
}
