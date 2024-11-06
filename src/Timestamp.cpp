#include <sstream>
#include <iomanip>

#include "Timestamp.h"

Timestamp Timestamp::now() {
    return Timestamp(std::chrono::system_clock::now());     
}
std::string Timestamp::toString() {
    // 转换为 time_t 类型以获取日期和时间
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(time_);

    // 将 time_t 转换为本地时间
    std::tm local_time = *std::localtime(&now_time_t);

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(time_.time_since_epoch()) % 1000;

    std::stringstream ss;
    // 输出日期和时间（格式化为 YYYY-MM-DD HH:MM:SS）
    ss << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S");

    // 输出毫秒部分
    ss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();

    return ss.str();
}