#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <type_traits>

class ThreadPool
{
public:
    ThreadPool(int threadNum);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&...args) 
    ->std::future<typename std::invoke_result<F, Args...>::type>
    {
        using ReturnType = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue after stop");
            }
            tasks_.emplace(
                [task]() { (*task)(); }
            );
        }
        cv_.notify_one();
        return result;
    }
private:
    std::vector<std::thread> threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_;
};
