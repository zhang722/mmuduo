#include <sys/timerfd.h>
#include <unistd.h>
#include <cstring>
#include <iterator>

#include "TimeQueue.h"
#include "Logger.h"
#include "EventLoop.h"


timespec howMuchTimeFromNow(Timestamp expiration) {
    auto now = std::chrono::system_clock::now();
    auto duration = expiration.time() - now;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() % 1000000000;

    struct timespec ts;
    ts.tv_sec = seconds;
    ts.tv_nsec = nanoseconds;
    return ts;
}

TimeQueue::TimeQueue(EventLoop* loop) 
  : loop_(loop),
    timerFd_(::timerfd_create(CLOCK_MONOTONIC,
                                TFD_NONBLOCK | TFD_CLOEXEC)),   
    timerChannel_(new Channel(loop, timerFd_))
{
    if (timerFd_ < 0) {
        LOG_FATAL("create timerfd failed.");
    }

    timerChannel_->setReadCallBack(
        std::bind(&TimeQueue::handleRead, this)
    );
    timerChannel_->enableReading();
}

TimeQueue::~TimeQueue() {
    timerChannel_->disableAll();
    loop_->removeChannel(timerChannel_.get());
    int n = ::close(timerFd_);
    if (n < 0) {
        LOG_FATAL("close timerfd failed.");
    }
}

void TimeQueue::addTimer(TimerCallback cb, double interval,
                         Timestamp expiration) {
    loop_->runInLoop(
        std::bind(&TimeQueue::addTimerInLoop, this, cb, interval, expiration)
    );
}

void TimeQueue::handleRead() {
    Timestamp now = Timestamp::now();
    uint64_t read_byte;
    ssize_t readn = ::read(timerFd_, &read_byte, sizeof(read_byte));
    
    if (readn != sizeof(read_byte)) {
        LOG_ERROR("TimerQueue::ReadTimerFd read_size < 0");
    }

    std::vector<Entry> expired = getExpired();
    for (auto& [time, timer] : expired) {
        timer->run();
    }
    resetExpiredTimers(expired, now);
}

std::vector<Entry> TimeQueue::getExpired() { 
    std::vector<Entry> expired; 
    for (auto it = expired.begin(); it != expired.end(); ++it) {
        *it;
    }

    Timestamp now = Timestamp::now();
    Entry sentry(now, nullptr);
    auto it = timers_.lower_bound(sentry);
    while (it != timers_.end() && it->first == now) {
        ++it;
    }

    for (auto iter = timers_.begin(); iter != it; ++iter) {
        expired.push_back(std::move(const_cast<Entry&>(*iter)));
    }

    timers_.erase(timers_.begin(), it);

    return expired;
}

void TimeQueue::resetExpiredTimers(const std::vector<Entry>& expired, Timestamp now) {
    for (auto&& [time, timer] : expired) {
        if (timer->repeat()) {
            timer->restart(now);
            timers_.emplace(timer->expiration(), std::move(const_cast<std::unique_ptr<Timer>&>(timer)));
        }
    }
    // 重置下一个到期的定时器
    if (!timers_.empty()) {
        resetFd(timers_.begin()->first);
    }
}

void TimeQueue::resetFd(Timestamp expiration) {
    // 使用 timerfd_settime() 唤醒事件循环
    itimerspec newValue;
    itimerspec oldValue;

    std::memset(&newValue, 0, sizeof newValue);
    std::memset(&oldValue, 0, sizeof oldValue);

    // 设置新的到期时间
    newValue.it_value = howMuchTimeFromNow(expiration);

    // 调用 timerfd_settime 以设置定时器
    int ret = ::timerfd_settime(timerFd_, 0, &newValue, &oldValue);
    if (ret < 0) {
        LOG_FATAL("timerfd_settime() error");
    }
}

void TimeQueue::addTimerInLoop(TimerCallback cb, double interval, Timestamp expiration) {
    auto timer = std::make_unique<Timer>(cb, interval, expiration);

    if (timers_.empty() || timers_.begin()->first > expiration) {
        resetFd(expiration);
    }

    timers_.insert(Entry(expiration, std::move(timer)));
}
