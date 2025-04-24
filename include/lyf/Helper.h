#pragma once

#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

namespace lyf {

using std::string, std::thread;

/// @brief 运行时断言函数, 若condition为false, 则抛出异常
/// @param condition 断言条件
/// @param what 异常信息
inline void
assure(bool condition, std::string_view what = "Assertion failed!") {
    if (!condition) {
        throw std::runtime_error{what.data()};
    }
}

/// @brief 获取当前时间戳
/// @return 当前时间戳
/// @note 单位为毫秒
inline int64_t
getCurrentTimeStamp() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// 获取当前时间的格式化字符串
// format 时间格式字符串, 默认为"%Y-%m-%d %H:%M:%S"
inline string
getCurrentTime(const string& format = "%Y-%m-%d %H:%M:%S") {
    time_t now = time(nullptr);
    char buf[1024];
    strftime(buf, sizeof(buf), format.c_str(), localtime(&now));
    return buf;
}

class Defer {
public:
    Defer(std::function<void()> func)
        : _func(func) {}

    ~Defer() {
        _func();
    }

private:
    std::function<void()> _func;
}; // class Defer

// 线程守卫类，用于确保线程在离开作用域时被正确地加入
class thread_guard {
public:
    explicit thread_guard(thread& t)
        : _t(t) {}

    ~thread_guard() {
        if (_t.joinable()) {
            _t.join();
        }
    }

public:
    thread_guard(const thread_guard&) = delete;
    thread_guard&
    operator=(const thread_guard&)
        = delete;

private:
    thread& _t;
}; // class thread_guard

} // namespace lyf
