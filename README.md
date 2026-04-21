# Web Server - 高并发HTTP服务器

一个从零开始实现的高并发网络服务器项目，通过逐步演化展示现代网络编程的核心概念。

## 🎯 项目目标

实现一个能处理 **10000+ 并发连接**的高性能HTTP服务器，最终达到：
- **QPS**: ~35000
- **并发连接数**: 10000+
- **架构**: Epoll ET + One Loop Per Thread + 业务线程池

## 📚 学习路径

### ✅ 完成的步骤

#### 第0步：基础阻塞式单连接服务器
- `socket → bind → listen → accept → read/write`
- 单个连接，演示基础网络编程
- **提交**: `ce0b29e`

#### 第1步：每连接一个线程
- 主线程 `accept`，每个连接创建新线程处理
- 支持多个并发连接
- **问题**: 线程数随连接数线性增长（不可扩展）
- **提交**: `e69e693 → d960a3f`（含模块化重构）

### 🚧 进行中的步骤

#### 第2步：线程池（待实现）
- 预先创建 N 个工作线程
- 所有连接共享这 N 个线程处理
- **目标**: 固定线程数，支持任意数量的连接

#### 第3步：Epoll 非阻塞 I/O（待实现）
- 用 `epoll(ET)` 替代阻塞的 `read/write`
- 一个线程处理多个连接的事件
- **关键**: ET模式下必须"读到EAGAIN、写到EAGAIN"

#### 第4步：One Loop Per Thread（待实现）
- 多个 `EventLoop` 线程
- 主 Reactor 只负责 `accept`
- 多个子 Reactor 分担连接处理
- **优势**: 减少竞争，提升吞吐

#### 第5步：业务线程池解耦（待实现）
- I/O 线程只负责网络读写
- 业务处理扔给专门的业务线程池
- 业务完成后投递回原 I/O 线程发送回复
- **优势**: I/O 不被业务卡住

## 🏗️ 项目结构

```
web_server/
├── CMakeLists.txt          # 编译配置
├── main.cpp                # 主程序入口
├── src/
│   ├── Connection.h        # 连接处理声明
│   ├── Connection.cpp      # 连接处理实现
│   └── ...                 # 后续模块
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

# 或用 curl（后续加HTTP支持后）
curl http://127.0.0.1:8080/
```

## 💡 核心概念速览

### 为什么需要多线程？
- 单线程：1000个连接，只能处理第一个，其他等死
- 每连接一线程：1000个连接 = 1000个线程，内存爆炸
- **解决**: 线程池 + 事件驱动 = 固定线程数 + 无限连接

### 为什么是 Epoll ET？
- `select/poll`: 每次轮询所有fd，O(n)
- `epoll LT`: 每次返回就绪fd，但没读完会重复通知
- `epoll ET`: 只通知一次，需要一次读到 EAGAIN，更高效

### 为什么要多个 EventLoop？
- 单个 EventLoop：所有连接争用一把锁
- 多个 EventLoop（One Loop Per Thread）：每个线程一个循环，无竞争

## 📖 面试说法

> "我实现了一个从单线程演化到高并发的网络服务器。从基础socket编程开始，逐步改造为：
> - 第1步：每连接一个线程（理解多线程基础）
> - 第2步：线程池（有限线程数复用）
> - 第3步：Epoll非阻塞（事件驱动）
> - 第4步：One Loop Per Thread（减少竞争）
> - 第5步：I/O与业务解耦（线程池）
> 
> 最终用 Epoll ET + 主从 Reactor + 业务线程池，实现 10000+ 并发连接，QPS 35000。"

## 🔗 参考

- Linux man pages: `socket`, `epoll`, `pthreads`
- 书籍: 《Linux高性能服务器编程》
- 开源项目: muduo, libevent, boost::asio

## 📝 进度追踪

| 步骤 | 目标 | 状态 | 提交 |
|------|------|------|------|
| 0 | 基础socket | ✅ 完成 | `ce0b29e` |
| 1 | 每连接一线程 | ✅ 完成 | `d960a3f` |
| 2 | 线程池 | 🚧 进行中 | - |
| 3 | Epoll非阻塞 | ⏳ 待做 | - |
| 4 | One Loop Per Thread | ⏳ 待做 | - |
| 5 | 业务线程池 | ⏳ 待做 | - |

---

**作者**: sevenstars2026  
**语言**: C++20  
**平台**: Linux  
**进度**: 20% (1/5 阶段完成)
