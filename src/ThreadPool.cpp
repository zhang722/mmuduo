#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadNum) {
    for (int i = 0; i < threadNum; i++) {
        threads_.emplace_back([this]() {
            while (true) {
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->mutex_);
                        cv_.wait(lock, [this]() {
                            return this->stop_ || !this->tasks_.empty();
                        });

                        if (this->stop_ || this->tasks_.empty()) {
                            return;
                        }

                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    }
                    task();
                }
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();

    for (auto&& t : threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}
