#pragma once

#include <memory>
#include <functional>
#include <string>

#include "InetAddress.h"
#include "Buffer.h"
#include "Socket.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using Callback = std::function<void()>;
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectCallback  = std::function<void(const TcpConnectionPtr&)>;
    using CloseCallback    = std::function<void(const TcpConnectionPtr&)>;
    using WriteCompleteCallback    = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback  = std::function<void(const TcpConnectionPtr&, Buffer *buf, Timestamp time)>;

    TcpConnection(const std::string& name, EventLoop *loop, int fd, const InetAddress&, const InetAddress&);
    ~TcpConnection() = default;

    const std::string& name() { return name_; }
    EventLoop* loop() { return loop_; }

    void setConnectionCallback(const ConnectCallback& cb) { connectCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    void send(Buffer *buf);
    void shutdown();

    void connectEstablished();
    void connectDestroyed();

private:
    enum State {
        Connecting,
        Disconnecting,
        Connected,
        Disconnected,
    };
    void handleRead(Timestamp time);
    void handleWrite();
    void handleClose();
    void handleError();

    void sendInLoop(const char* data, int len);
    void shutdownInLoop();

    void setState(State s);

    std::string name_;


    State state_;
    
    std::unique_ptr<Socket> socket_;
    const int fd_;
    Buffer inBuf_;
    Buffer outBuf_;
    EventLoop *loop_;
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    ConnectCallback connectCallback_;
    CloseCallback   closeCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    MessageCallback messageCallback_;
};

using TcpConnectionPtr = TcpConnection::TcpConnectionPtr;