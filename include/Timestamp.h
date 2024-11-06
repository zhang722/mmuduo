#pragma once

#include <chrono>
#include <string>

class Timestamp {
public:
    using time_point = std::chrono::time_point<std::chrono::system_clock>;
    explicit Timestamp(time_point time) : time_(time) {}

    static Timestamp now();
    std::string toString();
private:
    time_point time_;
};