#pragma once

#include <vector>

class Buffer {
public:
    Buffer(int len = INIT_LEN);
    // 不声明析构函数，编译器默认实现：拷贝构造，拷贝赋值，移动构造，移动赋值
    // ~Buffer() = default;

    char* peek() { return &*buf_.begin() + readIdx_; }
    int writeAvailable() { return buf_.size() - writeIdx_; }
    int readAvailable() { return writeIdx_ - readIdx_; }

    void append(const char *data, int len);
    void process(int len);
    ssize_t readFd(int fd);
    // 索引归零
    void reset();
private:
    static constexpr int INIT_LEN = 256;
    // 自动缩放Buffer最佳容量
    void optResize();

    // 将数据往前挪，至readIdx_=0
    void move();


    // Buffer最佳容量
    int meanSize_;
    int cnt_;

    int readIdx_;
    int writeIdx_;

    std::vector<char> buf_;
};