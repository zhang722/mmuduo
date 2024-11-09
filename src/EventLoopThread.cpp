#include "EventLoop.h"
#include "EventLoopThread.h"
#include "Logger.h"


EventLoopThread::EventLoopThread() 
  : ready_(false)
{
    std::thread temp(&EventLoopThread::threadFunc, this);
    thread_ = std::move(temp);
}

EventLoopThread::~EventLoopThread()
{
    if (thread_.joinable()) {
        thread_.join();  // 确保线程在析构时正确退出
    }
}

void EventLoopThread::threadFunc() {
    EventLoop loop;  // 将 loop 定义为线程的局部变量
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;  // 指向成员变量，生命周期同步
        ready_ = true;
        cv_.notify_one();  // 通知主线程 loop 已准备好
    }

    LOG_INFO("loop.loop() in thread: %d", std::this_thread::get_id());
    loop.loop();
}

EventLoop* EventLoopThread::startLoop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this]() { return ready_; });  // 等待 loop 准备好
    return loop_;  // 返回有效的 loop_ 指针
}