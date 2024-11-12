#include "Timer.h"
#include "Timestamp.h"

void Timer::restart(Timestamp now) {
    if (repeat_) {
        expiration_ = Timestamp(now.time() + std::chrono::milliseconds(static_cast<int>(interval_ * 1000)));
    } else {
        expiration_ = Timestamp();
    }
}