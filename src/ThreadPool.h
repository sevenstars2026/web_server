#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <cstdint>

namespace web {

// 线程池：管理固定数量的工作线程，分配任务给它们
class ThreadPool {
public:
    // 构造函数：创建指定数量的工作线程
    explicit ThreadPool(size_t numThreads = 4);
    
    // 析构函数：等待所有线程完成，然后销毁
    ~ThreadPool();
    
    // 删除拷贝构造和赋值操作
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    
    // 提交一个任务给线程池
    // 任务必须是可调用的（函数、lambda、函数对象等）
    template <typename Func>
    void submit(Func&& task) {
        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            taskQueue_.push(std::forward<Func>(task));
        }
        taskCV_.notify_one();  // 唤醒一个等待的工作线程
    }

private:
    // 工作线程的主循环：等待任务，执行任务，重复
    void workerLoop();

    size_t numThreads_;                                      // 工作线程数量
    std::vector<std::thread> workers_;                       // 工作线程数组
    std::queue<std::function<void()>> taskQueue_;            // 任务队列
    std::mutex taskMutex_;                                   // 保护任务队列
    std::condition_variable taskCV_;                         // 唤醒工作线程的信号
    bool shutting_down_ = false;                             // 线程池关闭标志
};

}  // namespace web
