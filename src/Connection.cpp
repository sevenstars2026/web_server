#include "Connection.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <string>

namespace web {

bool handleRead(Connection& conn) {
    char buf[4096];
    ssize_t n = ::read(conn.fd, buf, sizeof(buf));
    
    if (n < 0) {
        std::cerr << "[handleRead] read error from " << conn.clientIp << ":" << conn.clientPort << "\n";
        return false;
    }
    if (n == 0) {
        // 客户端关闭连接
        std::cout << "[handleRead] client " << conn.clientIp << ":" << conn.clientPort << " closed\n";
        return false;
    }

    // 数据放入读缓冲区
    conn.readBuffer.insert(conn.readBuffer.end(), buf, buf + n);
    std::cout << "[handleRead] received " << n << " bytes from " << conn.clientIp << "\n";

    // 简单协议：一条消息一行（\n 结尾）
    // 处理所有完整的行
    while (true) {
        auto it = std::find(conn.readBuffer.begin(), conn.readBuffer.end(), '\n');
        if (it == conn.readBuffer.end()) {
            break;  // 没有完整的消息
        }

        // 提取一条消息
        std::string message(conn.readBuffer.begin(), it);
        conn.readBuffer.erase(conn.readBuffer.begin(), it + 1);

        std::cout << "[handleRead] message: " << message << "\n";

        // 回复：echo 前面加上 "ECHO: "
        std::string reply = "ECHO: " + message + "\n";
        conn.writeBuffer.insert(conn.writeBuffer.end(), reply.begin(), reply.end());
    }

    return true;
}

bool handleWrite(Connection& conn) {
    if (conn.writeBuffer.empty()) {
        return true;  // 没有数据要写
    }

    ssize_t n = ::write(conn.fd, conn.writeBuffer.data(), conn.writeBuffer.size());
    
    if (n < 0) {
        std::cerr << "[handleWrite] write error to " << conn.clientIp << ":" << conn.clientPort << "\n";
        return false;
    }

    // 移除已发送的数据
    conn.writeBuffer.erase(conn.writeBuffer.begin(), conn.writeBuffer.begin() + n);
    std::cout << "[handleWrite] sent " << n << " bytes to " << conn.clientIp << "\n";

    return true;
}

}  // namespace web
