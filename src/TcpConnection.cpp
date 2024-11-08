#include "TcpConnection.h"
#include "Channel.h"
#include "Socket.h"
#include "EventLoop.h"


TcpConnection::TcpConnection(const std::string& name, 
                             EventLoop* loop, 
                             int fd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr) 
  : name_(name),
    state_(Connecting),
    loop_(loop),
    fd_(fd),
    socket_(new Socket(fd)),
    channel_(new Channel(loop, fd)),
    localAddr_(localAddr),
    peerAddr_(peerAddr)
{
    using namespace std::placeholders;
    channel_->setReadCallBack(std::bind(&TcpConnection::handleRead, this, _1));
    channel_->setWriteCallBack(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallBack(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallBack(std::bind(&TcpConnection::handleError, this));

    socket_->setKeepAlive(true);
}

void TcpConnection::send(Buffer* buf) {
    if (state_ != Connected) 
        return;

    if (loop_->isInLoopThread()) {
        sendInLoop(buf->peek(), buf->readAvailable());
        buf->reset();
    } else {
        size_t len = buf->readAvailable();
        std::string data(buf->peek(), len);

        auto temp = std::make_shared<Buffer>(std::move(*buf));
        loop_->queueInLoop([this, temp]() {
            this->sendInLoop(temp->peek(), temp->readAvailable());
        });
    }
}

void TcpConnection::shutdown() {
    if (state_ == Connected) {
        setState(Disconnecting);
        if (loop_->isInLoopThread()) {
            shutdownInLoop();
        } else {
            loop_->queueInLoop(
                std::bind(&TcpConnection::shutdownInLoop, this)
            );
        }
    }
}

void TcpConnection::connectEstablished() {
    // 该函数肯定在loop线程运行，因为只在newConnection()里调用过：
    // ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
    LOG_INFO("connection established.");

    channel_->tie(shared_from_this());
    channel_->enableReading();

    setState(Connected);
    if (connectCallback_)
        connectCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
    if (state_ == Connected) {
        setState(Disconnected);
        channel_->disableAll();
        if (connectCallback_)
            connectCallback_(shared_from_this());
    }
    loop_->removeChannel(channel_.get());
}

void TcpConnection::handleRead(Timestamp time) {
    ssize_t n = inBuf_.readFd(fd_);
    LOG_INFO("In TcpConnection::handleRead(): %d", n);

    if (n > 0) {
        LOG_INFO("n > 0, invoke callback.");
        if (messageCallback_) 
            messageCallback_(shared_from_this(), &inBuf_, time);
    }
    else if (n == 0) {
        LOG_INFO("n = 0, invoke handleClose.");
        handleClose();
    }
    else {
        LOG_INFO("n < 0, invoke handleError.");
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        ssize_t n = ::write(fd_, outBuf_.peek(), outBuf_.readAvailable());
        if (n > 0) {
            outBuf_.process(n);
            if (outBuf_.readAvailable() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_)
                    writeCompleteCallback_;
            } 
        } else {
            LOG_ERROR("handleWrite failed.");
        }
    }
}

void TcpConnection::handleClose() {
    setState(Disconnected);
    channel_->disableAll();
    if (connectCallback_)
        connectCallback_(shared_from_this());
    if (closeCallback_)
        closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    LOG_ERROR("In handleError.");
}

void TcpConnection::sendInLoop(const char* data, int len) {
    if (state_ != Connected)
        return;
    
    if (!channel_->isWriting() && outBuf_.readAvailable() == 0) {
        int n = ::write(fd_, data, len);
        if (n >= 0) {
            if (n == len && writeCompleteCallback_) {
                loop_->runInLoop(std::bind(writeCompleteCallback_, shared_from_this())); 
            }
        } else {
            outBuf_.append(data + n, len - n);
            if (!channel_->isWriting()) {
                channel_->enableWriting();
            }
            LOG_FATAL("write failed.");
        }
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdown();
    }
}

void TcpConnection::setState(State s) {
    state_ = s;
}
