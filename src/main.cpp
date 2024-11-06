#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include "Logger.h"
#include "Channel.h"
#include "EventLoop.h"

std::mutex m;
std::condition_variable cv;
EventLoop *loop = nullptr;

void otherThreadFunc() {
    EventLoop l;
    {
        std::unique_lock<std::mutex> lock(m); 
        loop = &l;
        cv.notify_all();
    }

    l.loop();
}

int main() {
    LOG_DEBUG("%s", "debug");
    LOG_INFO("%s", "info");
    LOG_ERROR("%s", "error");
    LOG_FATAL("%s", "fatal");



    LOG_INFO("main threadid: %d", std::this_thread::get_id());

    std::thread t(otherThreadFunc);

    {
        std::unique_lock<std::mutex> lock(m);
        cv.wait(lock, [&]{ return loop != nullptr; });
    }

    loop->runInLoop([]() {
        LOG_INFO("Functor passed from main.");
    });
    loop->wakeup();

    // std::this_thread::sleep_for(std::chrono::microseconds(1000));
    // loop->quit();
    

    if (t.joinable())
        t.join();
    return 0;
}