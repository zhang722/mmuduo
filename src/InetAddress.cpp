#include <arpa/inet.h>

#include "InetAddress.h"

InetAddress::InetAddress(const std::string ip, uint16_t port) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);
}
std::string InetAddress::toIp() {
    char *ip = inet_ntoa(addr_.sin_addr);
    return ip;
}
std::string InetAddress::toIpPort() {
    char *ip = inet_ntoa(addr_.sin_addr);
    int port = ntohs(addr_.sin_port);
    return std::string(ip) + std::to_string(port);
}
uint16_t InetAddress::toPort() {
    return ntohs(addr_.sin_port);
}


