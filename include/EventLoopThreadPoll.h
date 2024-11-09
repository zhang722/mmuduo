#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>

class EventLoop;
class EventLoopThread;

class EventLoopThreadPoll
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPoll(const std::string& name, EventLoop *loop, int threadNum);
    ~EventLoopThreadPoll();
    void start();
    EventLoop* getNextLoop();
private:
    std::string name_;
    EventLoop *baseLoop_;

    int threadNum_;
    int curIdx_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
    ThreadInitCallback threadInitCallback_;
};

