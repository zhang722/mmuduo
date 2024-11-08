#pragma once

#include <string>
#include <netinet/in.h>

class InetAddress {
public:
    explicit InetAddress(const std::string ip = "127.0.0.1", uint16_t port = 1111);
    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    std::string toIp();
    std::string toIpPort();
    uint16_t toPort();

    const sockaddr_in* getSockAddr() const { return &addr_; }
private:
    sockaddr_in addr_; 
};