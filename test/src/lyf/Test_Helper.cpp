#include "lyf/Helper.h"
#include <gtest/gtest.h>
#include <thread>

// 测试 assure 函数
TEST(HelperTest, AssureTest) {
    // 测试条件为 true 的情况
    EXPECT_NO_THROW(lyf::assure(true, "Test message"));

    // 测试条件为 false 的情况
    EXPECT_THROW(lyf::assure(false, "Test message"), std::runtime_error);
}

// 测试 getCurrentTimeStamp 函数
TEST(HelperTest, GetCurrentTimeStampTest) {
    int64_t timestamp = lyf::getCurrentTimeStamp();
    EXPECT_GT(timestamp, 0);
}

// 测试 getCurrentTime 函数
TEST(HelperTest, GetCurrentTimeTest) {
    std::string timeStr = lyf::getCurrentTime();
    EXPECT_FALSE(timeStr.empty());
}

// 测试 Defer 类
TEST(HelperTest, DeferTest) {
    bool flag = false;
    {
        lyf::Defer defer([&flag]() {
            flag = true;
        });
    }
    EXPECT_TRUE(flag);
}

// 测试 thread_guard 类
TEST(HelperTest, ThreadGuardTest) {
    std::thread t([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });

    { lyf::thread_guard guard(t); }

    EXPECT_FALSE(t.joinable());
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
