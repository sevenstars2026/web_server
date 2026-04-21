#include "ThreadPool.h"
#include <iostream>

namespace web {

ThreadPool::ThreadPool(size_t numThreads) : numThreads_(numThreads) {
    // 创建 numThreads个工作线程
    for (size_t i = 0; i < numThreads_; ++i) {
        // 每个工作线程运行 workerLoop()
        workers_.emplace_back([this] { this->workerLoop(); });
    }
    std::cout << "[ThreadPool] created " << numThreads_ << " worker threads\n";
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(taskMutex_);
        shutting_down_ = true;
    }
    // 唤醒所有工作线程，让它们看到 shutting_down_ = true 后退出
    taskCV_.notify_all();

    // 等待所有工作线程完成
    for (auto& worker : workers_) {
        worker.join();
    }
    std::cout << "[ThreadPool] destroyed\n";
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(taskMutex_);
            
            // 等待：直到有任务或线程池关闭
            taskCV_.wait(lock, [this] {
                return !taskQueue_.empty() || shutting_down_;
            });

            // 检查线程池是否关闭且没有任务了
            if (shutting_down_ && taskQueue_.empty()) {
                break;  // 退出循环，线程结束
            }

            // 从队列取出任务
            task = std::move(taskQueue_.front());
            taskQueue_.pop();
        }  // 解锁

        // 执行任务（在锁外执行，不会阻塞其他线程提交任务）
        task();
    }
}

}  // namespace web
