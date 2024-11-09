#pragma once

#include <thread>
#include <condition_variable>

#include "nocopyable.h"

class EventLoop;

class EventLoopThread : nocopyable {
public:
    EventLoopThread(/* args */);
    ~EventLoopThread();

    void threadFunc();
    EventLoop* startLoop();
private:
    EventLoop *loop_;
    std::thread thread_;
    std::mutex mutex_;
    bool ready_;
    std::condition_variable cv_;
};

