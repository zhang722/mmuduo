#include <iostream>

#include <cstdarg>

#include "Logger.h"
#include "Timestamp.h"


void Logger::log(LEVEL level, const char* format, ...) {
    if (level < level_) 
        return;

    std::lock_guard<std::mutex> lock(mutex_);

    char buf[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof buf, format, args);
    va_end(args);

    std::cout << '[' << level2str[level] << ']';
    std::cout << '[' << Timestamp::now().toString() << ']';
    std::cout << buf << std::endl;
}