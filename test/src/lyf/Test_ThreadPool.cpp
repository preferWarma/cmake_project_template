#include "lyf/ThreadPool.h"
#include <chrono>
#include <gtest/gtest.h>

// 测试线程池的初始化
TEST(ThreadPoolTest, Initialization) {
    lyf::ThreadPool& pool = lyf::ThreadPool::GetInstance();
    EXPECT_GT(pool.IdleThreadNum(), 0);
}

// 测试任务提交和执行
TEST(ThreadPoolTest, TaskCommitAndExecute) {
    lyf::ThreadPool& pool = lyf::ThreadPool::GetInstance();

    auto task = [](int a, int b) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return a + b;
    };

    auto future = pool.Commit(task, 2, 3);
    auto result = future.get();
    EXPECT_EQ(result, 5);
}

// 测试异常处理
TEST(ThreadPoolTest, ExceptionHandling) {
    lyf::ThreadPool& pool = lyf::ThreadPool::GetInstance();
    auto task             = []() {
        throw std::runtime_error("Test Exception");
    };
    auto future = pool.Commit(task);
    EXPECT_THROW(future.get(), std::runtime_error);
}

// 测试线程池的停止功能
TEST(ThreadPoolTest, StopThreadPool) {
    lyf::ThreadPool& pool = lyf::ThreadPool::GetInstance();
    pool.Stop();
    EXPECT_THROW(pool.Commit([]() {
        return 1;
    }),
                 std::runtime_error);
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
