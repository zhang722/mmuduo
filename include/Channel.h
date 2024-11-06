#pragma once

#include <memory>
#include <functional>
#include <linux/poll.h>

#include "nocopyable.h"
#include "Timestamp.h"

class TcpConnection;
class EventLoop;

enum {
    NONE_EVENT,
    WRITE,
    READ,
};

class Channel : nocopyable {
public:
    using CallBack = std::function<void()>;
    using ReadCallBack = std::function<void(Timestamp)>;

    explicit Channel(EventLoop *loop, int fd) : 
        loop_(loop), fd_(fd), addedToLoop_(false), tied_(false), event_(0), revent_(0) {}
    ~Channel() {};

    void tie(std::shared_ptr<TcpConnection>& tcpPtr);
    void handleEvent(Timestamp receiveTime);
    void setReadCallBack(ReadCallBack cb) { readCallBack_ = std::move(cb); }
    void setWriteCallBack(CallBack cb) { writeCallBack_ = std::move(cb); }
    void setCloseCallBack(CallBack cb) { closeCallBack_ = std::move(cb); }
    void setErrorCallBack(CallBack cb) { errorCallBack_ = std::move(cb); }

    void enableReading() { event_ |= ReadEvent; update(); }
    void disableReading() { event_ &= ~ReadEvent; update(); }
    void enableWriting() { event_ |= WriteEvent; update(); }
    void disableWriting() { event_ &= ~WriteEvent; update(); }
    void disableAll() { event_ = NoneEvent; update(); }
    bool isReading() { return event_ & ReadEvent; }
    bool isWriting() { return event_ & WriteEvent; }

    int fd() { return fd_; }
    int event() { return event_; }
    int& revent() { return revent_; }
private:
    static constexpr int NoneEvent = 0;
    static constexpr int ReadEvent = POLLIN;
    static constexpr int WriteEvent = POLLOUT;

    void update();
    void handleEventWithGuard(Timestamp receiveTime);

    EventLoop *loop_;
    bool addedToLoop_;
    int fd_;
    int event_;
    int revent_;
    std::weak_ptr<TcpConnection> tie_;
    bool tied_;

    ReadCallBack readCallBack_;
    CallBack writeCallBack_;
    CallBack closeCallBack_;
    CallBack errorCallBack_;
};