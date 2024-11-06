#pragma once

class nocopyable {
private:
    // 禁止拷贝
    nocopyable(const nocopyable&) = delete;
    nocopyable& operator=(const nocopyable&) = delete;
protected:
    // 只有被继承的时候有意义，因此设置成protected
    nocopyable() = default;
    ~nocopyable() = default;
};