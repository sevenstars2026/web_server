# Web Server - 基于 Epoll 的事件驱动网络服务器

一个从零开始实现的网络服务器项目，展示现代网络编程的核心概念。

## 🎯 项目描述

这是一个基于 **Linux Epoll** 实现的高效事件驱动网络服务器。采用单线程事件循环架构，可以高效处理大量并发连接。

## ✅ 已实现功能

### 基础网络功能
- **Socket 编程**：完整的 TCP 服务器实现
- **非阻塞 I/O**：所有连接使用非阻塞套接字
- **事件驱动**：Epoll 事件多路复用

### 核心架构
- **Epoll 管理**：`Epoll` 类封装 epoll 操作
- **连接管理**：`Connection` 结构存储每个连接的状态和缓冲区
- **事件循环**：单线程主循环处理所有连接事件

### 通信特性
- **简单协议**：基于行分隔的消息协议（`\n` 结尾）
- **Echo 服务**：接收消息后回复 `ECHO: <消息>`
- **缓冲区管理**：完整的读写缓冲区实现

## 🏗️ 项目结构

```
web_server/
├── CMakeLists.txt          # 编译配置
├── main.cpp                # 主程序入口，事件循环
├── src/
│   ├── Connection.h/.cpp   # 连接结构与事件处理
│   └── Epoll.h/.cpp        # Epoll 管理类
└── README.md               # 本文件
```

## 🔨 编译和运行

### 编译
```bash
cd /path/to/web_server
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug -j4
```

### 运行
```bash
./cmake-build-debug/web_server
# 监听 0.0.0.0:8080
```

### 测试
```bash
# 发送测试消息
echo "hello" | nc 127.0.0.1 8080
# 收到回复: ECHO: hello

# 多个并发连接
for i in {1..10}; do echo "msg$i" | nc 127.0.0.1 8080 & done
```

## 💡 设计要点

### Epoll 事件驱动
- 单线程事件循环通过 `epoll_wait()` 监听所有连接
- 有事件的连接才被唤醒处理
- 无需为每个连接分配线程

### 非阻塞 I/O 处理
- 所有 socket 设置为 `O_NONBLOCK`
- 读操作不会阻塞主线程
- 缓冲区存储未发送的数据

### 连接状态管理
- 每个连接拥有独立的读写缓冲区
- 通过 `epoll_event.data.fd` 关联 fd 与连接对象
- `std::map<int, Connection>` 存储所有活跃连接

## 📊 架构对比

| 方案 | 架构 | 最大连接数 | 内存占用 | 上下文切换 |
|------|------|--------|--------|---------|
| 阻塞 + 多线程 | N 个线程 | ~100 | 高 | 频繁 |
| **本项目** | **单线程 Epoll** | **10000+** | **低** | **很少** |

## 🔗 参考资料

- Linux man pages: `epoll(7)`, `epoll_ctl(2)`, `epoll_wait(2)`
- 书籍：《Linux高性能服务器编程》
- 开源项目：muduo, libevent, nginx

---

**语言**: C++20  
**平台**: Linux  
**依赖**: POSIX API (epoll)
