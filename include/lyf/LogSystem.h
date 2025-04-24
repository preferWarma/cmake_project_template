#pragma once

#include "Config.h"
#include "Helper.h"
#include "Singleton.h"
#include "StringTool.h"

#include <filesystem>
#include <fstream>
#include <ostream>
#include <queue>
#include <thread>

// 日志调用宏
#define LOG_ONLY(...)  lyf::AsyncLogSystem::GetInstance().LogOnly(__VA_ARGS__)
#define LOG_DEBUG(...) lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)  lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::WARN, __VA_ARGS__)
#define LOG_ERROR(...) lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::ERROR, __VA_ARGS__)

namespace lyf {

using std::atomic, std::condition_variable, std::mutex, std::string, std::thread;
using std::queue, std::ostream, std::ofstream;

class AsyncLogSystem : public Singleton<AsyncLogSystem> {
    friend class Singleton<AsyncLogSystem>;

public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
    };

    // 日志队列
    class LogQueue {
    public:
        void
        Push(const string& msg) {
            std::lock_guard<std::mutex> lock(_mutex);
            _que.push(msg);
            if (_que.size() == 1) {
                _cond.notify_one();
            }
        }

        bool
        Pop(string& msg) {
            std::unique_lock<mutex> lock(_mutex);
            _cond.wait(lock, [this] {
                return !_que.empty() || _isShutDown;
            });
            if (_isShutDown && _que.empty()) {
                return false;
            }
            msg = _que.front();
            _que.pop();
            return true;
        }

        void
        ShutDown() {
            _isShutDown = true;
            _cond.notify_all();
        }

    private:
        queue<string> _que;       // 日志队列
        mutex _mutex;             // 互斥锁
        condition_variable _cond; // 条件变量
        atomic<bool> _isShutDown; // 是否关闭
    };                            // class LogQueue

private:
    AsyncLogSystem()
        : _console(std::cout), _isShutDown(false) {
        string logFilePath = LOG_FILE_PATH;
        string logModeStr  = LOG_MODE;
        for (auto& ch : logModeStr) { // 转换为大写, 方便查找
            ch = std::toupper(ch);
        }
        bool toFile    = logModeStr.find("FILE") != string::npos;
        bool toConsole = logModeStr.find("CONSOLE") != string::npos;

        if (toFile) {
            // 提取目录和文件名
            std::filesystem::path logFilePathObj(logFilePath);
            std::filesystem::path logDir = logFilePathObj.parent_path();
            // 如果目录不存在，则创建目录
            if (!std::filesystem::exists(logDir)) {
                std::filesystem::create_directories(logDir);
            }

            _logFile.open(logFilePath, std::ios::out | std::ios::app);
            if (!_logFile.is_open()) {
                throw std::runtime_error("Failed to open log file: " + logFilePath);
            }
        }

        _worker = std::thread([this, toConsole, toFile]() -> void {
            string msg;
            while (_logQue.Pop(msg)) {
                if (toConsole) {
                    _console << msg << std::endl;
                }
                if (toFile) {
                    _logFile << msg << std::endl;
                }
            }
        });
    }

public:
    ~AsyncLogSystem() {
        _isShutDown = true;
        _logQue.ShutDown();
        if (_worker.joinable()) {
            _worker.join();
        }
        if (_logFile.is_open()) {
            _logFile.close();
        }
    }

public:
    template<typename... Args>
    void
    Log(LogLevel level, const string& fmt, Args&&... args) {
        if (_isShutDown) {
            return;
        }
        _logQue.Push(FormatMessage(level, fmt, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void
    LogOnly(const string& fmt, Args&&... args) {
        if (_isShutDown) {
            return;
        }
        _logQue.Push(lyf::FormatMessage(fmt, std::forward<Args>(args)...));
    }

private:
    inline string
    to_string(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG :
                return "DEBUG";
            case LogLevel::INFO :
                return "INFO";
            case LogLevel::WARN :
                return "WARN";
            case LogLevel::ERROR :
                return "ERROR";
            default :
                return "UNKNOWN";
        }
    }

    // 使用模板折叠格式化日志消息，支持 "{}" 占位符
    template<typename... Args>
    string
    FormatMessage(const LogLevel level, const string& fmt, Args&&... args) {
        std::ostringstream oss;
        oss << "[" << to_string(level) << "] " << getCurrentTime() << ": ";
        oss << lyf::FormatMessage(fmt, std::forward<Args>(args)...);
        return oss.str();
    }

private:
    LogQueue _logQue;         // 日志队列
    thread _worker;           // 工作线程
    ofstream _logFile;        // 日志输出文件
    ostream& _console;        // 日志输出流
    atomic<bool> _isShutDown; // 是否关闭
};                            // class AsyncLogSystem

} // namespace lyf
