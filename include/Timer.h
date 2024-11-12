#pragma once

#include <functional>

#include "nocopyable.h"
#include "Timestamp.h"

class Timer : nocopyable {
public:
    using TimerCallback = std::function<void()>;
    Timer(TimerCallback cb, double interval, Timestamp expiration)
      : timerCallback_(std::move(cb)),
        interval_(interval),
        expiration_(expiration),
        repeat_(interval > 0.0)
    {}

    void run() { 
        if (timerCallback_) {
            timerCallback_(); 
        }
    }
    double interval() { return interval_; }
    bool repeat() { return repeat_; }
    Timestamp expiration() { return expiration_; }

    void restart(Timestamp now);
private:
    TimerCallback timerCallback_;
    double interval_;
    bool repeat_;
    Timestamp expiration_;
};

using TimerCallback = Timer::TimerCallback;