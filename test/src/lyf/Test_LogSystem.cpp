#include "lyf/LogSystem.h"
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>

// 辅助函数：读取文件内容
std::string
read_file(const std::string& path) {
    std::ifstream file(path);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

// 辅助函数：重定向并捕获 std::cout 输出
std::string
capture_cout(const std::function<void()>& func) {
    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());
    func();
    std::cout.rdbuf(old);
    return buffer.str();
}

// 测试 fixture：用于重置日志系统状态
class LogSystemTest : public ::testing::Test {
protected:
    void
    TearDown() override {
        // 停止并重置日志系统（单例需要手动清理）
        lyf::AsyncLogSystem::GetInstance().Stop();
        // 注意：实际单例无法真正销毁，测试需保证独立性
    }
};

// 测试单例实例唯一性
TEST_F(LogSystemTest, SingletonInstance) {
    auto& instance1 = lyf::AsyncLogSystem::GetInstance();
    auto& instance2 = lyf::AsyncLogSystem::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

// 测试停止功能
TEST_F(LogSystemTest, StopFunction) {
    auto& logSystem = lyf::AsyncLogSystem::GetInstance();
    logSystem.Stop(); // 停止日志系统

    // 尝试发送日志（应被忽略）
    std::string output = capture_cout([&]() {
        LOG_FATAL("This should be ignored");
        logSystem.Flush(); // 刷新无效
    });

    EXPECT_TRUE(output.find("This should be ignored") == std::string::npos);
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // 设置测试临时目录（可选）
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    return RUN_ALL_TESTS();
}
