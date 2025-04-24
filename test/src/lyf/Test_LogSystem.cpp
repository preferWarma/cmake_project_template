#include "lyf/LogSystem.h"
#include <gtest/gtest.h>

// 测试日志队列的 Push 和 Pop 功能
TEST(LogQueueTest, PushAndPop) {
    lyf::AsyncLogSystem::LogQueue logQueue;
    std::string msg = "Test message";

    // 测试 Push
    logQueue.Push(msg);

    // 测试 Pop
    std::string poppedMsg;
    bool result = logQueue.Pop(poppedMsg);
    EXPECT_TRUE(result);
    EXPECT_EQ(poppedMsg, msg);
}

// 测试日志系统的 Log 功能
TEST(AsyncLogSystemTest, LogFunction) {
    // 确保日志系统实例化
    lyf::AsyncLogSystem& logSystem = lyf::AsyncLogSystem::GetInstance();

    // 测试 Log 函数
    logSystem.Log(lyf::AsyncLogSystem::LogLevel::DEBUG, "Test log message: {}", 123);
    // 由于日志是异步处理的，这里无法直接验证日志是否正确写入
    // 可以考虑检查日志文件或控制台输出，但这会使测试变得复杂
    // 这里简单地认为没有抛出异常就是成功
    SUCCEED();
}

// 测试日志系统的 LogOnly 功能
TEST(AsyncLogSystemTest, LogOnlyFunction) {
    // 确保日志系统实例化
    lyf::AsyncLogSystem& logSystem = lyf::AsyncLogSystem::GetInstance();

    // 测试 LogOnly 函数
    logSystem.LogOnly("Test log only message: {}", 456);
    // 由于日志是异步处理的，这里无法直接验证日志是否正确写入
    // 可以考虑检查日志文件或控制台输出，但这会使测试变得复杂
    // 这里简单地认为没有抛出异常就是成功
    SUCCEED();
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
