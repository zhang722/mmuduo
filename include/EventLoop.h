#pragma once

#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

#include "nocopyable.h"
#include "TimeQueue.h"

class Poller;
class Channel;

class EventLoop : nocopyable {
public:
    using ChannelList = std::vector<Channel*>;
    using Functor = std::function<void()>;
    using FunctorList = std::vector<Functor>;

    EventLoop();
    ~EventLoop();

    void loop();
    // 其他线程调用接口
    void quit(); 

    // 其他线程调用接口
    void wakeup();

    // 其他线程调用接口
    void addOrUpdateChannel(Channel *channel);
    // 其他线程调用接口
    void removeChannel(Channel *channel);
    // 其他线程调用接口
    bool hasChannel(Channel *channel);

    bool isInLoopThread() {
        return std::this_thread::get_id() == threadId_;
    }

    // 其他线程调用接口
    void runInLoop(Functor cb);
    // 其他线程调用接口
    void queueInLoop(Functor cb);

    // 定时器
    void runAt(Functor&& cb, Timestamp expiration) {
        timeQueue_->addTimer(std::move(cb), 0.0, expiration);
    }

    void runAfter(Functor&& cb, double waitTimeS) {
        Timestamp time(Timestamp::now().time() + std::chrono::milliseconds(static_cast<int>(waitTimeS * 1000)));
        timeQueue_->addTimer(std::move(cb), 0.0, time);
    }

    void runEvery(Functor&& cb, double intervalS) {
        Timestamp time(Timestamp::now().time() + std::chrono::milliseconds(static_cast<int>(intervalS * 1000)));
        timeQueue_->addTimer(std::move(cb), intervalS, time);     
    }

private:
    static constexpr int TIMEOUT = 10000;
    void handleRead();
    void doPendingFunctors();

    std::atomic<bool> quit_;

    int64_t iteration_;
    std::thread::id threadId_;

    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimeQueue> timeQueue_;

    int wakeupFd_;
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    Channel *currentChannel_;

    std::mutex mutex_;
    FunctorList functors_;
};