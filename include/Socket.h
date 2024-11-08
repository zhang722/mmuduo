#pragma once

#include <unistd.h>

#include "nocopyable.h"
#include "InetAddress.h"
#include "Logger.h"

class Socket : nocopyable {
public:
    Socket(int fd) : fd_(fd) {}
    Socket(const InetAddress& addr);
    ~Socket() { 
        if (::close(fd_) < 0)
            LOG_FATAL("Close fd: %d failed.", fd_); 
    }
    void setKeepAlive(bool on);
    void listen();
    int accept(InetAddress *peerAddr, int *error);
    void shutdown();
    int fd() { return fd_; }

    void setReuseAddr(bool on);
    void setReusePort(bool on);

private:

    int fd_;
};