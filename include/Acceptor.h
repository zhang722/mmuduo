#pragma once

#include <memory>
#include <unordered_set>
#include <functional>

#include "nocopyable.h"
#include "InetAddress.h"
#include "Socket.h"
#include "TcpConnection.h"

// class Socket;
class Channel;
class EventLoop;

class Acceptor : nocopyable {
public:
    using NewConnectionCallback = std::function<void()>;
    Acceptor(EventLoop *loop, const InetAddress& listenAddr, bool reusePort);

    void setNewConnectionCallback(NewConnectionCallback cb) {
        newConnectionCallback_ = std::move(cb);
    }
    void newConnection(int fd, const InetAddress& peerAddr);

private:
    void handleRead();

    EventLoop *loop_;
    int fd_;
    InetAddress listenAddr_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    int idleFd_;

    NewConnectionCallback newConnectionCallback_;

    std::unordered_set<TcpConnection::TcpConnectionPtr> connections_;    
};