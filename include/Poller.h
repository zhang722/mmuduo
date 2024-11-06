#pragma once

#include <vector>
#include <unordered_map>

#include "nocopyable.h"
#include "Channel.h"

class Timestamp;
class EventLoop;
struct epoll_event;

class Poller : nocopyable {
public: 
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop) : loop_(loop) {}
    virtual ~Poller() = default;

    virtual void addOrUpdateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;
    virtual Timestamp poll(int timeout, ChannelList *activeChannels) = 0;
    virtual bool hasChannel(Channel * channel) const { 
        return static_cast<bool>(channels_.count(channel->fd()));
    }
    void assertInLoopThread() const {
        // loop_->assertInLoopThread();
    }

protected:
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop *loop_;
};