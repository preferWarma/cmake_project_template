#pragma once

#include "Config.h"
#include "Helper.h"
#include "Singleton.h"
#include "StringTool.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ostream>
#include <queue>
#include <sstream>
#include <thread>
#include <utility>

// 日志调用宏
#define LOG_DEBUG(...) lyf::AsyncLogSystem::GetInstance().Debug(__VA_ARGS__)
#define LOG_INFO(...)  lyf::AsyncLogSystem::GetInstance().Info(__VA_ARGS__)
#define LOG_WARN(...)  lyf::AsyncLogSystem::GetInstance().Warn(__VA_ARGS__)
#define LOG_ERROR(...) lyf::AsyncLogSystem::GetInstance().Error(__VA_ARGS__)
#define LOG_FATAL(...) lyf::AsyncLogSystem::GetInstance().Fatal(__VA_ARGS__)

namespace lyf {

using std::atomic, std::condition_variable, std::mutex, std::string, std::thread;
using std::queue, std::ostream, std::ofstream, std::lock_guard;

class AsyncLogSystem : public Singleton<AsyncLogSystem> {
    friend class Singleton<AsyncLogSystem>;

public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
    };

    struct LogMessage {
        LogLevel level;                             // 日志级别
        string content;                             // 日志内容
        std::chrono::system_clock::time_point time; // 日志时间

        LogMessage(LogLevel level, string content)
            : level(level), content(std::move(content)), time(std::chrono::system_clock::now()) {}
    };

    // 日志队列
    class LogQueue {
        using milliseconds = std::chrono::milliseconds;

    public:
        using QueueType = queue<LogMessage>;

    public:
        LogQueue(size_t maxSize = 10000)
            : maxSize(maxSize), _isStop(false) {}

        // 生产端：添加日志消息（优化锁粒度）
        void
        Push(const LogMessage& msg) {
            {
                std::lock_guard<std::mutex> lock(_queMtx);
                _currentQueue.push(std::move(msg));
                // 如果当前队列已满, 则交换当前队列和备用队列
                if (_currentQueue.size() >= maxSize) {
                    std::swap(_currentQueue, _backupQueue);
                }
            }
            // 如果当前队列从空变为非空, 则通知消费者线程
            _notEmpty.notify_one();
        }

        // 消费端：获取所有日志消息（添加超时机制）
        bool
        PopAll(QueueType& output, milliseconds timeout = milliseconds(100)) {
            std::unique_lock<mutex> lock(_queMtx);
            _notEmpty.wait_for(lock, timeout, [this] {
                return !_currentQueue.empty() || _isStop;
            });

            // 如果当前队列为空且停止, 则返回false
            if (_currentQueue.empty() && _isStop) {
                return false;
            }
            // 如果当前队列不为空, 则交换当前队列和备用队列
            if (!_currentQueue.empty()) {
                _backupQueue.swap(_currentQueue);
            }
            output.swap(_backupQueue);
            return true;
        }

        void
        Stop() {
            _isStop = true;
            _notEmpty.notify_all();
        }

    private:
        QueueType _currentQueue;      // 当前生产队列
        QueueType _backupQueue;       // 备用生产队列
        mutable mutex _queMtx;        // 互斥锁(用于交换当前队列和备用队列)
        condition_variable _notEmpty; // 条件变量, 用于等待队列有数据
        atomic<bool> _isStop;         // 是否关闭
        size_t maxSize;               // 最大队列大小
    };                                // class LogQueue

private:
    AsyncLogSystem()
        : _logFilePath(LOG_FILE_PATH), _isStop(false), _logQue(10000) {
        _outputToFile    = LOG_MODE.find("FILE") != string::npos;
        _outputToConsole = LOG_MODE.find("CONSOLE") != string::npos;

        if (_outputToFile) {
            if (!CreateLogDirectory(_logFilePath)) {
                std::cerr << "Failed to create log directory." << std::endl;
                _outputToFile = false;
            } else {
                // 以追加模式打开文件, 如果文件不存在则创建
                _logFile.open(_logFilePath, std::ios::app | std::ios::out);
                if (!_logFile.is_open()) {
                    std::cerr << "Failed to open log file: " << _logFilePath << std::endl;
                    _outputToFile = false;
                }
            }
        }
        _worker = thread([this]() {
            WorkerLoop();
        });
    }

public:
    ~AsyncLogSystem() {
        // 停止日志系统
        Stop();
    }

public:
    template<typename... Args>
    void
    Log(LogLevel level, const string& fmt, Args&&... args) {
        if (_isStop) {
            return;
        }
        _logQue.Push(LogMessage(level, FormatMessage(fmt, std::forward<Args>(args)...)));
    }

    // 停止日志系统
    inline void
    Stop() {
        // 如果已经停止, 则直接返回
        if (_isStop.exchange(true)) {
            return;
        }

        _logQue.Stop(); // 通知工作线程停止
        Flush();        // 刷新日志文件

        // 等待工作线程完成
        if (_worker.joinable()) {
            _worker.join();
        }

        // 关闭日志文件
        if (_outputToFile) {
            lock_guard<mutex> lock(_fileMtx);
            if (_logFile.is_open()) {
                _logFile << "[" << getCurrentTime() << "] [SYSTEM] Log system closed." << std::endl;
                _logFile.flush(); // 确保日志写入文件
                _logFile.close();
            }
        }
        // 输出系统关闭信息到控制台
        if (_outputToConsole) {
            // 红色字体
            std::cout << "\033[1;31m"
                      << "[" << getCurrentTime() << "] [SYSTEM] Log system closed."
                      << "\033[0m" << std::endl;
        }
    }

    // 刷新日志文件
    inline void
    Flush() {
        LogQueue::QueueType batchQueue; // 当前批次的日志消息队列
        while (_logQue.PopAll(batchQueue)) {
            // 处理当前批次的日志消息
            ProcessBatch(batchQueue);
            batchQueue = LogQueue::QueueType(); // 清空批次队列
        }

        // 刷新日志文件
        if (_outputToFile) {
            lock_guard<mutex> lock(_fileMtx);
            if (_logFile.is_open()) {
                _logFile.flush();
            }
        }
    }

    // 便捷操作
    template<typename... Args>
    inline void
    Debug(const string& fmt, Args&&... args) {
        Log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void
    Info(const string& fmt, Args&&... args) {
        Log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void
    Warn(const string& fmt, Args&&... args) {
        Log(LogLevel::WARN, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void
    Error(const string& fmt, Args&&... args) {
        Log(LogLevel::ERROR, fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    inline void
    Fatal(const string& fmt, Args&&... args) {
        Log(LogLevel::FATAL, fmt, std::forward<Args>(args)...);
    }

    string
    GetOutputFilePath() const {
        // 获取完整的绝对路径
        return std::filesystem::absolute(_logFilePath).string();
    }

private:
    inline string
    LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG :
                return "DEBUG";
            case LogLevel::INFO :
                return "INFO ";
            case LogLevel::WARN :
                return "WARN ";
            case LogLevel::ERROR :
                return "ERROR";
            case LogLevel::FATAL :
                return "FATAL";
            default :
                return "UNKNOWN";
        }
    }

    inline string
    LevelColor(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG :
                return "\033[0;37m"; // 白色
            case LogLevel::INFO :
                return "\033[0;32m"; // 绿色
            case LogLevel::WARN :
                return "\033[1;33m"; // 黄色
            case LogLevel::ERROR :
                return "\033[1;31m"; // 红色
            case LogLevel::FATAL :
                return "\033[1;35m"; // 紫色
            default :
                return "\033[0m";    // 默认
        }
    }

    inline bool
    CreateLogDirectory(const string& path) {
        try {
            auto dir = std::filesystem::path(path).parent_path();
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to create log directory: " << e.what() << std::endl;
            return false;
        }
    }

    // 工作线程函数
    inline void
    WorkerLoop() {
        LogQueue::QueueType batchQueue; // 当前批次的日志消息队列
        while (!_isStop.load(std::memory_order_relaxed)) {
            if (_logQue.PopAll(batchQueue)) {
                // 处理当前批次的日志消息
                ProcessBatch(batchQueue);
                // 清空当前批次的日志消息队列
                batchQueue = LogQueue::QueueType();
            }
        }
        // 此时_isStop为true, 处理剩余的日志消息
        while (_logQue.PopAll(batchQueue)) {
            ProcessBatch(batchQueue);
            batchQueue = LogQueue::QueueType();
        }
        // 等待工作线程完成
        // 日志系统关闭信息写入日志文件
        // 系统析构时会执上述行此操作
    }

    // 处理当前批次的日志消息
    inline void
    ProcessBatch(LogQueue::QueueType& batchQueue) {
        // 处理当前批次的日志消息
        std::stringstream consoleStream;
        std::stringstream fileStream;
        while (!batchQueue.empty()) {
            LogMessage msg = std::move(batchQueue.front());
            batchQueue.pop();

            string timeStr    = formatTime(msg.time);
            string levelStr   = LevelToString(msg.level);
            string resetColor = "\033[0m"; // 重置颜色

            consoleStream << LevelColor(msg.level) << "[" << timeStr << "] "
                          << "[" << levelStr << "] " << msg.content << "\033[0m" << std::endl;

            fileStream << "[" << timeStr << "] "
                       << "[" << levelStr << "] " << msg.content << std::endl;
        }

        // 输出到控制台
        if (_outputToConsole) {
            // 控制台输出时, 不需要锁保护, 因为输出的是单个日志消息
            std::cout << consoleStream.str() << std::flush; // 立即刷新
        }
        // 输出到文件
        if (_outputToFile) {
            lock_guard<mutex> lock(_fileMtx);
            if (_logFile.is_open()) {
                _logFile << fileStream.str() << std::flush;
            }
        }
    }

private:
    LogQueue _logQue;       // 日志队列
    thread _worker;         // 工作线程
    ofstream _logFile;      // 日志输出文件
    string _logFilePath;    // 日志文件路径
    atomic<bool> _isStop;   // 是否关闭
    mutable mutex _fileMtx; // 用于文件操作的互斥锁
    bool _outputToFile;     // 是否输出到文件
    bool _outputToConsole;  // 是否输出到控制台
};                          // class AsyncLogSystem

} // namespace lyf
