#include <arpa/inet.h>

#include "Socket.h"
#include "Logger.h"

Socket::Socket(const InetAddress& addr) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0) 
        LOG_FATAL("Create socket failed.");
    
    int n = ::bind(fd_, 
                   reinterpret_cast<sockaddr*>(const_cast<sockaddr_in*>(addr.getSockAddr())),
                   sizeof(sockaddr_in));
    if (n < 0)
        LOG_FATAL("Bind socket failed.");
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    int n = ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                         static_cast<socklen_t>(sizeof optval));
    if (n < 0)
        LOG_FATAL("setKeepAlive failed.");
}

void Socket::listen() {
    int ret = ::listen(fd_, SOMAXCONN);
    if (ret < 0)
      LOG_FATAL("listen failed.");
}

int Socket::accept(InetAddress *peerAddr, int* error) {
    sockaddr addr;
    socklen_t len = static_cast<socklen_t>(sizeof(addr));
    int fd = ::accept4(fd_, &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd < 0) {
        LOG_FATAL("accept failed.");
        *error = errno;
    }
      
    return fd;
}

void Socket::shutdown() {
    if (::shutdown(fd_, SHUT_WR))
        LOG_FATAL("shutdown failed.");
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    int n = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
    if (n < 0)
        LOG_FATAL("Socket setReuseAddr failed.");
}

void Socket::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    int n = ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT,
                         &optval, static_cast<socklen_t>(sizeof optval)); 
    if (n < 0)
        LOG_FATAL("Socket setReusePort failed.");
}