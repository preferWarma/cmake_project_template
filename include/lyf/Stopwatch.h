#pragma once

#include <chrono>
#include <iostream>

namespace lyf {

/// @brief 计时器类声明
class stopwatch {
    using system_clock = std::chrono::system_clock;
    using time_point   = system_clock::time_point;

protected:
    bool started{false};   // 是否已经开始计时
    bool stopped{false};   // 是否已经停止计时
    double rate{1.f};      // 时间比例(默认为1us)
    time_point begin_time; // 开始时间
    time_point end_time;   // 停止时间
    size_t tick{0ull};     // duration的tick数(纳秒)

public:
    enum class TimeType {
        ns = 1,
        us = 1000,
        ms = 1000000,
        s  = 1000000000
    };

    // 指定rate倍数的ns作为单位
    stopwatch(double rate = 1.0)
        : started(false),
          stopped(false),
          rate(rate),
          tick(0),
          begin_time(system_clock::now()),
          end_time(system_clock::now()) {}

    // 指定时间类型作为单位
    stopwatch(TimeType type)
        : stopwatch(static_cast<double>(type)) {}

    virtual ~stopwatch() = default;

    inline bool
    is_started() const {
        return started;
    }

    inline void
    start() {
        reset();
        started    = true;
        stopped    = false;
        begin_time = system_clock::now();
    }

    inline void
    stop() {
        if (!started) {
            return;
        }
        stopped  = true;
        end_time = system_clock::now();
        tick = static_cast<size_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count());
    }

    inline void
    reset() {
        started = false;
        stopped = false;
        tick    = 0;
    }

    inline double
    duration() {
        // 未启动计时器, 抛出异常
        if (!started) {
            throw std::runtime_error("StopWatch::duration(): Not started yet.");
            return -1;
        }
        // 未停止计时器, 停止计时器计算duration后再启动
        if (!stopped) {
            stop();
            stopped = true;
        }
        return static_cast<double>(tick) / rate; // 返回单位为rate倍的ns
    }
};                                               // class StopWatch

// 对TimeType的输出运算符重载
inline std::ostream&
operator<<(std::ostream& os, stopwatch::TimeType type) {
    switch (type) {
        case stopwatch::TimeType::ns :
            os << "ns";
            break;
        case stopwatch::TimeType::us :
            os << "us";
            break;
        case stopwatch::TimeType::ms :
            os << "ms";
            break;
        case stopwatch::TimeType::s :
            os << "s";
            break;
        default :
            break;
    }
    return os;
}

class auto_stopwatch : public stopwatch {
public:
    inline auto_stopwatch(double rate = 1.0)
        : stopwatch(rate) {
        this->start();
    }

    inline auto_stopwatch(TimeType type)
        : stopwatch(type) {
        this->start();
    }

    inline ~auto_stopwatch() {
        std::cout << "duration time: " << this->duration() << static_cast<TimeType>(rate) << std::endl;
    }
}; // class auto_stopWatch

} // namespace lyf
