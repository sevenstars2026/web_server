#pragma once

#include <sys/epoll.h>
#include <cstdint>

namespace web {

// Epoll 管理器：处理事件驱动的 IO
class Epoll {
public:
    Epoll();
    ~Epoll();

    // 删除拷贝操作
    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    // 添加文件描述符到 epoll，监听事件
    // events: EPOLLIN (可读), EPOLLOUT (可写), 或两者的组合
    void addFd(int fd, uint32_t events);

    // 修改文件描述符的监听事件
    void modFd(int fd, uint32_t events);

    // 从 epoll 移除文件描述符
    void removeFd(int fd);

    // 等待事件发生，返回有事件的 fd 数量
    // events: 存储返回的事件数组
    // maxEvents: 最多返回多少个事件
    // timeout: 超时时间(毫秒)，-1 表示阻塞等待
    int wait(epoll_event* events, int maxEvents, int timeout = -1);

private:
    int epfd_;  // epoll 文件描述符
};

}  // namespace web
