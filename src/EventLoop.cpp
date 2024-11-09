#include <sys/eventfd.h>
#include <unistd.h>
#include <cassert>

#include "EventLoop.h"
#include "DefaultPoller.h"
#include "Channel.h"
#include "Logger.h"
#include "Timestamp.h"

EventLoop::EventLoop()
  : quit_(false),
    iteration_(0),
    threadId_(std::this_thread::get_id()),
    poller_(defaultPoller(this)),
    wakeupFd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
    wakeupChannel_(new Channel(this, wakeupFd_)),
    currentChannel_(nullptr)
{

    if (wakeupFd_ < 0)
        LOG_FATAL("Create wakeFd_ failed.");
    wakeupChannel_->setReadCallBack(std::bind(&EventLoop::handleRead, this));
    wakeupChannel_->enableReading();
    assert(hasChannel(wakeupChannel_.get()));
}

EventLoop::~EventLoop() {
    ::close(wakeupFd_);
}

void EventLoop::loop() {
    LOG_INFO("threadid: %d", std::this_thread::get_id());
    while (!quit_) {
        activeChannels_.clear();
        Timestamp time = poller_->poll(TIMEOUT, &activeChannels_);

        for (auto channel : activeChannels_) {
            currentChannel_ = channel;
            channel->handleEvent(time);
        }
        currentChannel_ = nullptr;

        LOG_INFO("Waked up or timeout or something happened.");
        doPendingFunctors();
    }
}

void EventLoop::quit() {
    LOG_INFO("quit from threadid: %d", std::this_thread::get_id());
    quit_ = true;
    if (!isInLoopThread()) 
        wakeup();
}

void EventLoop::wakeup() {
    LOG_INFO("Invoke wakeup: %d", std::this_thread::get_id());
    uint64_t u = 1;
    if (write(wakeupFd_, &u, sizeof(uint64_t)) != sizeof(u)) 
        LOG_FATAL("Loop can not wake up.");
}

void EventLoop::addOrUpdateChannel(Channel *channel) {
    if (!isInLoopThread()) {
        LOG_FATAL("Modified loop in other thread.");
        return;
    }

    poller_->addOrUpdateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    if (!isInLoopThread()) {
        LOG_FATAL("Modified loop in other thread.");
        return;
    }

    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) { 
    return poller_->hasChannel(channel);
}

void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {
        LOG_INFO("Invoke runInLoop, isInloopThread: %d", std::this_thread::get_id());
        cb();
    } else {
        LOG_INFO("Invoke runInLoop, not In loopThread: %d", std::this_thread::get_id());
        queueInLoop(std::move(cb));
    } 
}


void EventLoop::handleRead() {
    // 一定在eventloop的线程
    uint64_t buf = 0;
    ssize_t n = read(wakeupFd_, &buf, sizeof(buf));
    LOG_INFO("read in handleRead: %d", n);
    if (n != sizeof(buf))
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
}

void EventLoop::queueInLoop(Functor cb) {
    LOG_INFO("queueInLoop in thread: %d", std::this_thread::get_id());
    {
        std::lock_guard<std::mutex> g(mutex_);
        functors_.push_back(std::move(cb));
    }
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::doPendingFunctors() {
    FunctorList functors;
    {
        std::lock_guard<std::mutex> g(mutex_);
        functors.swap(functors_);
    }

    for (auto f : functors) {
        f();
    }
}
