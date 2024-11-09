#include <functional>

#include "TcpServer.h"
#include "Acceptor.h"
#include "EventLoopThreadPoll.h"
#include "EventLoop.h"

TcpServer::TcpServer(EventLoop* loop, 
                     const InetAddress& listenAddr,
                     const std::string& name, 
                     bool reusePort,
                     int threadNum) 
  : name_(name),
    threadNum_(threadNum),
    acceptor_(new Acceptor(loop, listenAddr, reusePort)),
    threadPoll_(new EventLoopThreadPoll(name, loop, threadNum))
{
    using namespace std::placeholders;
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2)
    );
}

TcpServer::~TcpServer() {
    for (auto c : connections_) {
        TcpConnectionPtr conn = c.second;
        conn->loop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn)
        );
    }
}

void TcpServer::start() {
    threadPoll_->start();
}

void TcpServer::newConnection(int fd, const InetAddress& peerAddr) {
    std::string name = "conn" + std::to_string(fd);
    EventLoop *loop = threadPoll_->getNextLoop();
    TcpConnection::TcpConnectionPtr conn(new TcpConnection(name,
                                                            loop,
                                                            fd,
                                                            acceptor_->getListenAddr(),
                                                            peerAddr)); 
    
    if (connectCallback_) {
        conn->setConnectionCallback(connectCallback_);
    }

    if (messageCallback_) {
        conn->setMessageCallback(messageCallback_);
    }

    if (writeCompleteCallback_) {
        conn->setWriteCompleteCallback(writeCompleteCallback_);
    }

    using namespace std::placeholders;
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this, _1)
    );

    connections_[name] = conn;
    LOG_INFO("Tcpconnection: %s is created.", name);
    loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    connections_.erase(conn->name());

    conn->loop()->runInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn)
    );
}
