#pragma once

#include <cstdint>

namespace web {

// 处理单个客户端连接的函数
// 在独立的线程中运行
void handleConnection(int connfd, const char* clientIp, uint16_t clientPort);

}  // namespace web
