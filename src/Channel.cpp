#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"


void Channel::tie(std::shared_ptr<TcpConnection>& tcpPtr) {
    tie_ = tcpPtr;
    tied_ = true;
}

void Channel::update() {
    addedToLoop_ = true;
    loop_->addOrUpdateChannel(this);
}


void Channel::handleEvent(Timestamp receiveTime) {
    // 根据tied_判断是不是一个TcpConnection的channel
    // TcpConnection在创建的时候，会设置channel的tied_=true;
    if (tied_) {
        // TcpConnection已经消亡，可能是用户那边把TcpConnectionPtr释放了
        if (tie_.lock()) {
            handleEventWithGuard(receiveTime);
        }
    } 
    // 这里的else代表：这是一个wakeupChannel
    else {
        handleEventWithGuard(receiveTime);
    }
}


void Channel::handleEventWithGuard(Timestamp receiveTime) {
    if ((revent_ & POLLHUP) && !(revent_ & POLLIN)) {
        if (closeCallBack_) 
            closeCallBack_();
    }

    if (revent_ & (POLLERR | POLLNVAL)) {
        if (errorCallBack_)
            errorCallBack_();
    }

    if (revent_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        LOG_INFO("Read in handleEvent.");
        if (readCallBack_)
            readCallBack_(receiveTime);
    }

    if (revent_ & POLLOUT) {
        if (writeCallBack_)
            writeCallBack_();
    }
}