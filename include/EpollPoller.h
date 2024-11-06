#include "Poller.h"
#include "nocopyable.h"

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop *loop);
    ~EpollPoller() override;

    void addOrUpdateChannel(Channel *channel) override;
    void removeChannel(Channel *channel) override;
    Timestamp poll(int timeout, ChannelList *activeChannels) override;

private:
    using EventList = std::vector<epoll_event>;
    static constexpr int INIT_NUM = 16;

    int epollfd_;
    EventList events_;
};