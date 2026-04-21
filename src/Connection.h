#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace web {

// 连接结构：存储一个客户端连接的状态
struct Connection {
    int fd = -1;                        // socket fd
    char clientIp[16] = {};             // 客户端 IP
    uint16_t clientPort = 0;            // 客户端端口
    std::vector<char> readBuffer;       // 读缓冲区
    std::vector<char> writeBuffer;      // 写缓冲区
    
    Connection() = default;
    
    // 禁止拷贝
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    
    // 允许移动
    Connection(Connection&& other) noexcept 
        : fd(other.fd), clientPort(other.clientPort) {
        std::strcpy(clientIp, other.clientIp);
        readBuffer = std::move(other.readBuffer);
        writeBuffer = std::move(other.writeBuffer);
        other.fd = -1;
    }
    
    Connection& operator=(Connection&& other) noexcept {
        if (this != &other) {
            fd = other.fd;
            clientPort = other.clientPort;
            std::strcpy(clientIp, other.clientIp);
            readBuffer = std::move(other.readBuffer);
            writeBuffer = std::move(other.writeBuffer);
            other.fd = -1;
        }
        return *this;
    }
};

// 处理连接的可读事件：尝试从 fd 读数据
// 返回 true 表示成功，false 表示需要关闭连接
bool handleRead(Connection& conn);

// 处理连接的可写事件：尝试向 fd 写缓冲区的数据
// 返回 true 表示成功，false 表示需要关闭连接
bool handleWrite(Connection& conn);

}  // namespace web
