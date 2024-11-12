#pragma once

#include <memory>
#include <vector>
#include <set>

#include "Channel.h"
#include "Timer.h"

class EventLoop;

using Entry = std::pair<Timestamp, std::unique_ptr<Timer>>;


struct CompareEntry {
    bool operator()(const Entry& lhs, const Entry& rhs) const {
        if (!(lhs.first == rhs.first)) {
            return lhs.first < rhs.first;
        }
        // 比较 unique_ptr 的地址
        return lhs.second.get() < rhs.second.get();
    }
};


class TimeQueue
{
public:
    using TimerList = std::set<Entry, CompareEntry>;
    TimeQueue(EventLoop *loop);
    ~TimeQueue();
    void addTimer(TimerCallback cb, double interval, Timestamp expiration);
    
private:
    void handleRead();
    std::vector<Entry> getExpired();
    void resetExpiredTimers(const std::vector<Entry>&, Timestamp now);

    void resetFd(Timestamp expiration);

    void addTimerInLoop(TimerCallback cb, double interval, Timestamp expiration);

    EventLoop *loop_;
    int timerFd_;
    std::unique_ptr<Channel> timerChannel_;
    TimerList timers_;
};
