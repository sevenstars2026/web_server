#include "Connection.h"

#include <iostream>
#include <thread>
#include <unistd.h>

namespace web {

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

}  // namespace web
