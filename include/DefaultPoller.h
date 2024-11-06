#pragma once

#include "Poller.h"
#include "EpollPoller.h"

Poller* defaultPoller(EventLoop *loop) {
    return new EpollPoller(loop);
}