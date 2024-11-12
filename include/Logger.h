#pragma once

#include <unordered_map>
#include <string>
#include <mutex>

#include "nocopyable.h"


#define LOG_DEBUG(...) Logger::instance().log(Logger::DEBUG,  __VA_ARGS__)
#define LOG_INFO(...) Logger::instance().log(Logger::INFO,  __VA_ARGS__)
#define LOG_ERROR(...) Logger::instance().log(Logger::ERROR,  __VA_ARGS__)
#define LOG_FATAL(...) Logger::instance().log(Logger::FATAL,  __VA_ARGS__)

#define SET_LOG_LEVEL(...) Logger::instance().setLogLevel(__VA_ARGS__)

class Logger : nocopyable  {
public:
    enum LEVEL {
        DEBUG,
        INFO,
        ERROR,
        FATAL,
    };
    static Logger& instance() {
        static Logger logger;
        return logger;
    }
    LEVEL logLevel() {
        return level_;
    }

    void setLogLevel(LEVEL level) {
        level_ = level;
    }

    void log(LEVEL level, const char* format, ...);
private:
    Logger() : level_(LEVEL::INFO) {}
    LEVEL level_;
    std::unordered_map<LEVEL, std::string> level2str = {
        {LEVEL::DEBUG, "DEBUG"},
        {LEVEL::INFO, "INFO"},
        {LEVEL::ERROR, "ERROR"},
        {LEVEL::FATAL, "FATAL"},
    };
    std::mutex mutex_;
};