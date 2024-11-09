#pragma once

#include <memory>
#include <unordered_set>
#include <functional>

#include "nocopyable.h"
#include "InetAddress.h"
#include "Socket.h"
#include "Channel.h"
#include "TcpConnection.h"

class EventLoop;

class Acceptor : nocopyable {
public:
    using NewConnectionCallback = std::function<void(int, const InetAddress&)>;
    Acceptor(EventLoop *loop, const InetAddress& listenAddr, bool reusePort);

    void setNewConnectionCallback(NewConnectionCallback cb) {
        newConnectionCallback_ = std::move(cb);
    }

    InetAddress getListenAddr() { return listenAddr_; }

private:
    void handleRead();

    EventLoop *loop_;
    int fd_;
    InetAddress listenAddr_;
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    int idleFd_;

    NewConnectionCallback newConnectionCallback_;
};