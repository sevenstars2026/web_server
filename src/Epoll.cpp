#include "Epoll.h"
#include <iostream>
#include <unistd.h>

namespace web {

Epoll::Epoll() {
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epfd_ < 0) {
        std::cerr << "epoll_create1 failed\n";
    } else {
        std::cout << "[Epoll] created, fd=" << epfd_ << "\n";
    }
}

Epoll::~Epoll() {
    if (epfd_ >= 0) {
        close(epfd_);
        std::cout << "[Epoll] destroyed\n";
    }
}

void Epoll::addFd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev) < 0) {
        std::cerr << "[Epoll] add fd " << fd << " failed\n";
    } else {
        std::cout << "[Epoll] added fd " << fd << "\n";
    }
}

void Epoll::modFd(int fd, uint32_t events) {
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;

    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        std::cerr << "[Epoll] mod fd " << fd << " failed\n";
    }
}

void Epoll::removeFd(int fd) {
    if (epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr) < 0) {
        std::cerr << "[Epoll] remove fd " << fd << " failed\n";
    } else {
        std::cout << "[Epoll] removed fd " << fd << "\n";
    }
}

int Epoll::wait(epoll_event* events, int maxEvents, int timeout) {
    return epoll_wait(epfd_, events, maxEvents, timeout);
}

}  // namespace web
