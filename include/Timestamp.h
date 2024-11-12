#pragma once

#include <chrono>
#include <string>

class Timestamp {
public:
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    explicit Timestamp(TimePoint time = TimePoint()) : time_(time) {}

    static Timestamp now();
    std::string toString();
    TimePoint time() { return time_; }

    // Comparison operators
    bool operator<(const Timestamp& other) const {
        return time_ < other.time_;
    }
    bool operator>(const Timestamp& other) const {
        return time_ > other.time_;
    }
    bool operator==(const Timestamp& other) const {
        return time_ == other.time_;
    }
private:
    TimePoint time_;
};