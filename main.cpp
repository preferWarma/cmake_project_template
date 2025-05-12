#include "lyf/LogSystem.h"

// 测试代码示例（伪代码）
// 测试代码示例（伪代码）
int
main() {
    // 初始化日志系统（输出到文件和控制台）
    lyf::AsyncLogSystem::GetInstance();

    // 发送小流量日志（未达maxSize）
    for (int i = 0; i < 10; ++i) {
        LOG_INFO("Test log message: {}", i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 模拟间隔
    }

    // 发送大流量日志（超过maxSize）
    for (int i = 0; i < 20000; ++i) {
        if (i % 1000 == 0) {
            LOG_INFO("High volume log: {}", i);
        } else {
            LOG_DEBUG("High volume log: {}", i);
        }
    }

    try {
        // 模拟抛出异常
        throw std::runtime_error("Test exception");
    } catch (const std::exception& e) {
        // 捕获异常并记录
        LOG_ERROR("Exception caught: {}", e.what());
    }

    // 主动停止日志系统（触发Flush）
    lyf::AsyncLogSystem::GetInstance().Stop();
    return 0;
}
