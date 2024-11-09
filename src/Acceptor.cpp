#include <fcntl.h>

#include "Acceptor.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "TcpConnection.h"

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort)
  : loop_(loop),
    listenAddr_(listenAddr),
    socket_(new Socket(listenAddr)),
    channel_(new Channel(loop, socket_->fd())),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    socket_->setReuseAddr(true);
    socket_->setReusePort(reusePort);
    channel_->setReadCallBack(
        std::bind(&Acceptor::handleRead, this)
    );

    socket_->listen();
    channel_->enableReading();
}


void Acceptor::handleRead() {
    InetAddress peerAddr;
    int error = -1;
    int connfd = socket_->accept(&peerAddr, &error);
    if (connfd >= 0) {
        LOG_INFO("A new listened.");
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        }
        else 
            ::close(connfd);
    } else {
        if (error == EMFILE) {
            ::close(idleFd_);
            idleFd_ = ::accept(socket_->fd(), NULL, NULL);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

