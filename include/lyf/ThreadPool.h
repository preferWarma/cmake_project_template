#pragma once
#include <atomic>
#include <condition_variable>
#include <exception>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Singleton.h"

namespace lyf {

using std::atomic, std::condition_variable, std::future, std::thread;
using std::mutex, std::queue, std::unique_lock, std::vector;

class ThreadPool : public Singleton<ThreadPool> {
    friend class Singleton<ThreadPool>;

public:
    ThreadPool(size_t threadNum = std::thread::hardware_concurrency())
        : _stop(false), _idleThreadNum(threadNum > 0 ? threadNum : 1) {
        Init();
    }

    ~ThreadPool() {
        Stop();
    }

public:
    using TaskType = std::packaged_task<void()>;

public:
    size_t
    IdleThreadNum() {
        return _idleThreadNum.load();
    }

    template<class F, class... Args>
    auto
    Commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using RetType = decltype(f(args...));
        if (_stop.load()) {
            throw std::runtime_error("commit on stopped ThreadPool");
        }
        auto task = std::make_shared<std::packaged_task<RetType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<RetType> ret = task->get_future();
        {
            // 对任务队列加锁
            std::lock_guard<mutex> lock(_mtx);
            _taskQueue.emplace([task]() {
                try {
                    (*task)();
                } catch (std::exception& e) { // 此时的异常是 task 抛出的
                    throw e;                  // 向上抛出异常，此时抛出的是匿名函数捕获的异常
                }
            });
        }
        // 唤醒一个线程执行该任务
        _cv.notify_one();
        return ret;
    }

    void
    Init() {
        for (size_t i = 0; i < _idleThreadNum; ++i) {
            _threads.emplace_back([this]() {
                while (!_stop.load()) {
                    TaskType task;
                    {
                        std::unique_lock<mutex> lock(_mtx);
                        _cv.wait(lock, [this]() {
                            return _stop.load() || !_taskQueue.empty();
                        });
                        if (_taskQueue.empty()) {
                            return;
                        }
                        task = std::move(_taskQueue.front());
                        _taskQueue.pop();
                    }
                    // 使用原子操作减少竞态条件
                    --_idleThreadNum;
                    try {
                        task();                   // 执行任务
                    } catch (std::exception& e) { // 此时捕获的是执行 task 的匿名函数抛出的异常
                        throw e;                  // 向上抛出异常，此时的异常需被外部捕获处理
                    }
                    ++_idleThreadNum;
                }
            });
        }
    }

    void
    Stop() {
        _stop.store(true);
        _cv.notify_all(); // 唤醒所有线程
        for (auto& thread : _threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    atomic<bool> _stop;            // 线程池是否停止
    atomic<size_t> _idleThreadNum; // 空闲线程数量
    mutex _mtx;                    // 队列的互斥锁
    condition_variable _cv;        // 条件阻塞
    queue<TaskType> _taskQueue;    // 任务队列
    vector<thread> _threads;       // 线程池
};                                 // class ThreadPool

} // namespace lyf
