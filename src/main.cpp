#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

#include "Logger.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "EventLoopThreadPoll.h"
#include "TcpServer.h"
#include "ThreadPool.h"

int main() {
    SET_LOG_LEVEL(Logger::LEVEL::FATAL);
    LOG_INFO("main threadid: %d", std::this_thread::get_id());
    EventLoop loop;
    InetAddress listenAddr;
    TcpServer server(&loop, listenAddr, "server", false, 3);
    server.setConnectCallback([](const TcpConnectionPtr& conn) {
        std::cout << "conn :" << conn->name() << "created." << std::endl;
    });
    server.setMessageCallback([](const TcpConnectionPtr& conn, Buffer *buf, Timestamp time) {
        conn->send(buf);
    });
    server.setWriteCompleteCallback([](const TcpConnectionPtr& conn) {
        std::cout << "conn: " << conn->name() << std::endl;
    });

    loop.runEvery([]() {
        LOG_INFO("run every");
    }, 2.0);

    ThreadPool threadPool(3);    
    auto result = threadPool.enqueue([]() {
        LOG_INFO("task1");
        return 1;
    });

    LOG_INFO("%d", result.get());

    server.start();
    loop.loop();

    return 0;
}