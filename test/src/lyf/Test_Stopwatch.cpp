#include "lyf/Stopwatch.h"
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

namespace lyf {

// 测试 stopwatch 类的 start 和 stop 方法
TEST(StopwatchTest, StartAndStop) {
    stopwatch sw(stopwatch::TimeType::us);
    sw.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sw.stop();
    double duration = sw.duration();
    EXPECT_GT(duration, 0);
}

// 测试 stopwatch 类的 reset 方法
TEST(StopwatchTest, Reset) {
    stopwatch sw(stopwatch::TimeType::us);
    sw.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    sw.stop();
    sw.reset();
    EXPECT_FALSE(sw.is_started());
    EXPECT_THROW(sw.duration(), std::runtime_error);
}

// 测试 auto_stopwatch 类的自动计时功能
TEST(AutoStopwatchTest, AutoDuration) {
    {
        auto_stopwatch asw(stopwatch::TimeType::ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // 由于 auto_stopwatch 在析构时输出时间，这里只需要确保没有异常抛出
}

// 测试 stopwatch 类未启动时调用 duration 方法抛出异常
TEST(StopwatchTest, DurationWithoutStart) {
    stopwatch sw(stopwatch::TimeType::us);
    EXPECT_THROW(sw.duration(), std::runtime_error);
}

} // namespace lyf

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
