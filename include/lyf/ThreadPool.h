#pragma once
#include <atomic>
#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace lyf {

using std::atomic, std::condition_variable, std::future, std::thread;
using std::mutex, std::queue, std::unique_lock, std::vector;

class ThreadPool {
public:
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool&
    operator=(const ThreadPool&)
        = delete;

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
    static ThreadPool&
    Instance() {
        static ThreadPool instance;
        return instance;
    }

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
                (*task)();
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
                    task(); // 执行任务
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
