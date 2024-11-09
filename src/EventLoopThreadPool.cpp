#include "EventLoopThread.h"
#include "EventLoopThreadPoll.h"

EventLoopThreadPoll::EventLoopThreadPoll(const std::string& name,
                                         EventLoop* loop, int threadNum)
  : name_(name),
    baseLoop_(loop),
    threadNum_(threadNum),
    curIdx_(-1)
{

}

EventLoopThreadPoll::~EventLoopThreadPoll() {}

void EventLoopThreadPoll::start() {
    for (int i = 0; i < threadNum_; i++) {
        std::unique_ptr<EventLoopThread> t = std::unique_ptr<EventLoopThread>(new EventLoopThread());
        EventLoop *loop = t->startLoop();
        threads_.push_back(std::move(t));
        loops_.push_back(loop);
    }
}

EventLoop* EventLoopThreadPoll::getNextLoop() {
    EventLoop *loop = baseLoop_;
    if (threads_.size() > 0) {
        curIdx_++;
        if (curIdx_ == threads_.size()) {
            curIdx_ = 0;
        }
        loop = loops_[curIdx_];
    }

    return loop;
}
