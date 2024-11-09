#pragma once

#include <memory>
#include <string>
#include <map>

#include "EventLoopThreadPoll.h"
#include "TcpConnection.h"
#include "Acceptor.h"

class EventLoop;
class InetAddress;

class TcpServer
{
public:
    using ConnectCallback  = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback  = std::function<void(const TcpConnectionPtr&, Buffer *buf, Timestamp time)>;
    using WriteCompleteCallback    = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback    = std::function<void(const TcpConnectionPtr&)>;

    TcpServer(EventLoop* loop, 
              const InetAddress& listenAddr, 
              const std::string& name, 
              bool reusePort = false,
              int threadNum = 0);
    ~TcpServer();
    void start();

    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setConnectCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }

private:
    void newConnection(int fd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

    std::string name_;
    int threadNum_;

    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPoll> threadPoll_;
    std::map<std::string, TcpConnectionPtr> connections_;

    ConnectCallback connectCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
};

