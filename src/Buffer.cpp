#include <cstring>
#include <sys/uio.h>

#include "Buffer.h"
#include "Logger.h"

Buffer::Buffer(int len)
  : meanSize_(0),
    cnt_(0),
    readIdx_(0),
    writeIdx_(0),
    buf_(len)
{

}

void Buffer::append(const char* data, int len) {
    if (writeAvailable() < len)
        move();

    if (writeAvailable() < len) {
        buf_.resize((writeIdx_ + len) * 2);
    }
    std::memcpy(&buf_[writeIdx_], data, len);
    writeIdx_ += len;
    // 每次append就计算最优容量
    optResize();
}

void Buffer::process(int len) {
    if (len < readAvailable()) 
        readIdx_ += len;
    else 
        reset();
}

ssize_t Buffer::readFd(int fd) {
    char extrabuf[65536];
    iovec iov[2];
    int writeBytes = writeAvailable();

    iov[0].iov_base = &buf_[writeIdx_];
    iov[0].iov_len = writeBytes;
    iov[1].iov_base = extrabuf;
    iov[1].iov_len = sizeof(extrabuf);
    
    int num_iov = sizeof(extrabuf) <= writeBytes ? 1 : 2;
    ssize_t bytes = readv(fd, iov, num_iov);
    if (bytes < 0) {
        LOG_FATAL("readv return < 0");
        return bytes;
    }

    if (static_cast<int>(bytes) <= writeBytes) {
        writeIdx_ += bytes; 
    } else {
        writeIdx_ = buf_.size();
        append(extrabuf, bytes - writeBytes);
    }

    return bytes;
}

void Buffer::optResize() {
    cnt_++;
    meanSize_ += static_cast<int>(static_cast<double>(buf_.size()) / static_cast<double>(cnt_));

    if (meanSize_ * 2 < buf_.size() && meanSize_ > writeIdx_)
        buf_.resize(meanSize_);
}

void Buffer::move() {
    int len = readAvailable();
    if (len == 0)
        return;

    std::memmove(&buf_[0], &buf_[readIdx_], len);    
    readIdx_ = 0;
    writeIdx_ = len;
}

void Buffer::reset() {
    readIdx_ = writeIdx_ = 0;
}
