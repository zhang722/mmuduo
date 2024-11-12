#include <sys/epoll.h>
#include <unistd.h>

#include "EpollPoller.h"
#include "Logger.h"

EpollPoller::EpollPoller(EventLoop *loop) 
  : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(INIT_NUM)
{
    if (epollfd_ < 0) 
        LOG_FATAL("Create epollfd failed."); 
}

EpollPoller::~EpollPoller() {
    ::close(epollfd_);
}

void EpollPoller::addOrUpdateChannel(Channel *channel) {
    int fd = channel->fd();
    epoll_event event;
    event.events = channel->event();
    event.data.fd = fd;
    if (!hasChannel(channel)) {
        if (::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &event) < 0) 
            LOG_FATAL("EPOLL CTL ADD failed.");
        channels_[fd] = channel;
    } else {
        if (::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &event) < 0)
            LOG_FATAL("EPOLL CTL MOD failed.");
    }
}

void EpollPoller::removeChannel(Channel *channel) {
    int fd = channel->fd();
    if (!hasChannel(channel)) {
        LOG_FATAL("Remove channel failed.");
        return;
    }

    epoll_event event;
    event.events = channel->event();
    if (::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &event) < 0) 
        LOG_FATAL("EPOLL CTL DEL failed.");
    channels_.erase(fd);
}


Timestamp EpollPoller::poll(int timeout, ChannelList *activeChannels) {
    int num = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), -1);

    if (num > 0) {
        LOG_INFO("num: %d, events size: %d", num, events_.size());
        for (int i = 0; i < num; i++) {
            Channel *c = channels_[events_[i].data.fd];
            
            // if (!hasChannel(c)) {
            //     LOG_INFO("Doesn't have channel.");
            //     continue;
            // }
            c->revent() = events_[i].events; 
            activeChannels->push_back(c);
        }
        if (num >= events_.size())
            events_.resize(num * 2);
    } else if (num == 0) {
        LOG_INFO("Nothing.");
    } else {
        LOG_FATAL("Epoll error.");
    }

    return Timestamp::now();
}
