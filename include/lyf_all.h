#pragma once

#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <cxxabi.h>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <mutex>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// 用于获取变量类型的快捷调用宏, 此方法会忽略掉没有显示指明的默认参数类型,
// 例如template<class T, class U = string> class A {};则对于auto a = A<Color>();将返回A<Color>而不是A<Color, string>,
// 若要获取完整的类型, 可以使用getWholeTypeName(x)方法
#define typeof(x) lyf::getTypeName<decltype(x)>()

// 用于获取变量类型的快捷调用宏, 此方法会获取完整的类型, 包括默认参数类型
#define getWholeTypeName(x) lyf::type_class<decltype(x)>::get()

// 用于快捷打印变量名和值的宏
#define m_print(arg) std::cout << #arg << " = " << arg << std::endl
// 快捷输出并附带文件行号定位
#define m_debug(...) std::cout << "[" << __FILE__ << ":" << __LINE__ << "]: " << __VA_ARGS__ << std::endl

// 日志调用宏
#define LOG_ONLY(...)  lyf::AsyncLogSystem::GetInstance().LogOnly(__VA_ARGS__)
#define LOG_DEBUG(...) lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...)  lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...)  lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::WARN, __VA_ARGS__)
#define LOG_ERROR(...) lyf::AsyncLogSystem::GetInstance().Log(lyf::AsyncLogSystem::LogLevel::ERROR, __VA_ARGS__)

namespace lyf {

using std::atomic;
using std::cout, std::endl;
using std::mutex, std::lock_guard, std::condition_variable;
using std::string, std::vector, std::queue;
using std::thread;

const std::string LOG_FILE_PATH = "./log/log.txt";
const std::string LOG_MODE      = "CONSOLE";

/// 采用模板参数的方式, 基于宏__PRETTY_FUNCTION__获取类型, 此方法不依赖于typeid关键字,
/// 调用方法为lyf::getTypeName<decltype(param)>() 此方法获取的名称更加简洁直观, 但会忽略掉默认的参数类型;
/// 例如template<class T, class U = string> class A {};则对于auto a = A<Color>();此方法将返回A<Color>而不是A<Color,
/// string> 若需要获取完整的类型, 可以使用getWholeTypeName(x)方法
template<typename T>
string
getTypeName() {
    string s  = __PRETTY_FUNCTION__;
    auto pos1 = s.find("T = ") + 4; // +4是为了跳过"T = "这几个字符
    auto pos2 = s.find_first_of("	]", pos1);
    return s.substr(pos1, pos2 - pos1 + 1);
}

template<typename Helper>
struct cvr_saver {}; // 用于保存变量的const/volatile/reference属性

// 采用模板参数而不是函数参数的方式获取变量类型获取param的类型的调用格式为lyf::type_class<decltype(param)>::get()
template<typename T>
class type_class {
public:
    // 获取变量类型(依赖于typeid关键字)
    static string
    get() {
        string all_realName = string{
            abi::__cxa_demangle(typeid(cvr_saver<T>).name(), nullptr, nullptr, nullptr)}; // 包含cuv_saver结构体的全名
        auto pos1 = all_realName.find_first_of('<') + 1;                                  // 第一个'<'后的位置
        auto pos2 = all_realName.find_last_of('>');                                       // 最后一个'>'的位置
        return all_realName.substr(pos1, pos2 - pos1);                                    // 去掉干扰信息
    }
};                                                                                        // class type_class

/// @brief 仿python的split函数, 分隔符为char
/// @param str 要分隔的字符串
/// @param delim 分隔符
/// @return 分隔后的字符串数组, 以vector<string>形式返回
inline vector<string>
split(const string& str, const char delim) {
    std::stringstream ss(str);
    string s;
    vector<string> res;
    res.clear();
    while (getline(ss, s, delim)) {
        res.push_back(s);
    }
    return res;
}

/// @brief 仿python的split函数, 分隔符为string
/// @param str 要分隔的字符串
/// @param delim 分隔符
/// @return 分隔后的字符串数组, 以vector<string>形式返回
inline vector<string>
split(const string& str, const string& delim) {
    size_t pos1 = 0;
    size_t pos2 = str.find_first_of(delim, pos1); // 查找第一个分隔符的位置
    vector<string> res;
    while (string::npos != pos2) {
        res.push_back(str.substr(pos1, pos2 - pos1));
        pos1 = pos2 + delim.size();
        pos2 = str.find(delim, pos1);
    }
    if (pos1 != str.size()) {
        res.push_back(str.substr(pos1));
    }
    return res;
}

/// @brief 以正则表达式匹配字符串
/// @param str 要匹配的字符串
/// @param pattern 要匹配的正则表达式
/// @return 匹配后的字符串数组, 以vector<string>形式返回
inline vector<string>
regex_match(const string& str, const string& pattern) {
    using std::regex, std::smatch, std::sregex_iterator;
    regex m_pattern{pattern};
    auto word_begin = sregex_iterator(str.begin(), str.end(), m_pattern);
    auto word_end   = sregex_iterator();
    vector<string> res;
    for (auto i = word_begin; i != word_end; ++i) {
        smatch match = *i;
        res.emplace_back(match.str());
    }
    return res;
}

/// @brief 替换字符串中的第一个指定子串
/// @param str 要替换的字符串
/// @param old_value 要替换的子串
/// @param new_value 替换后的子串
/// @return 替换后的字符串
inline string
replace_first(const string& str, const string& old_value, const string& new_value) {
    string res = str;
    auto pos   = res.find(old_value);
    if (pos != string::npos) {
        return res.replace(pos, old_value.length(), new_value);
    } else {
        return str;
    }
}

/// @brief 替换字符串中的所有指定子串
/// @param str 要替换的字符串
/// @param old_value 要替换的子串
/// @param new_value 替换后的子串
/// @return 替换后的字符串
inline string
replace_all(const string& str, const string& old_value, const string& new_value) {
    string res = str;
    for (size_t pos = 0; pos != string::npos; pos += new_value.length()) {
        pos = res.find(old_value, pos);
        if (pos != string::npos) {
            res.replace(pos, old_value.length(), new_value);
        } else {
            break;
        }
    }
    return res;
}

/// @brief 替换字符串中的最后一个指定子串
/// @param str 要替换的字符串
/// @param old_value 要替换的子串
/// @param new_value 替换后的子串
/// @return 替换后的字符串
inline string
replace_last(const string& str, const string& old_value, const string& new_value) {
    string res = str;
    auto pos   = res.rfind(old_value);
    if (pos != string::npos) {
        return res.replace(pos, old_value.length(), new_value);
    } else {
        return str;
    }
}

/// @brief 判断字符串是否以指定前缀开头
/// @param str 要判断的字符串
/// @param prefix 前缀字符串
/// @return 是否以指定前缀开头
inline bool
begin_with(const string& str, const string& prefix) {
    for (size_t i = 0; i < prefix.size(); ++i) {
        if (str[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

/// @brief 判断字符串是否以指定后缀结尾
/// @param str 要判断的字符串
/// @param suffix 后缀字符串
/// @return 是否以指定后缀结尾
inline bool
end_with(const string& str, const string& suffix) {
    size_t str_len    = str.size();
    size_t suffix_len = suffix.size();
    if (str_len < suffix_len) {
        return false;
    }
    for (size_t i = 0; i < suffix_len; ++i) {
        if (str[str_len - suffix_len + i] != suffix[i]) {
            return false;
        }
    }
    return true;
}

/// @brief 以迭代器方式遍历容器元素
/// @param v 要遍历的容器
/// @param delim 每个元素之间的分隔符
/// @param os 输出流(默认为cout)
template<typename T>
void
print_container(const T& v, const string& delim = " ", std::ostream& os = cout) {
    for (const auto& i : v) {
        os << i << delim;
    }
    os << endl;
}

/// @brief 范围内遍历打印容器元素
/// @tparam Iter 迭代器类型
/// @param first 开始迭代器(包含)
/// @param last 结束迭代器(不包含)
/// @param delim 每个元素之间的分隔符
/// @param os 输出流(默认为cout)
template<typename Iter>
void
print_container(Iter first, Iter last, const string& delim = " ", std::ostream& os = cout) {
    using T = typename std::iterator_traits<Iter>::value_type;
    std::copy(first, last, std::ostream_iterator<T>(os, delim.c_str()));
    os << endl;
}

inline string
blue(const string& str) {
    return "\033[34m" + str + "\033[0m";
}

inline string
green(const string& str) {
    return "\033[32m" + str + "\033[0m";
}

inline string
red(const string& str) {
    return "\033[31m" + str + "\033[0m";
}

template<class T1, class T2>
auto
max(T1&& a, T2&& b) {
    return a > b ? std::forward<T1>(a) : std::forward<T2>(b);
}

template<class T, class... Args>
auto
max(T&& a, Args&&... args) {
    return max(std::forward<T>(a), max(std::forward<Args>(args)...));
}

template<class T1, class T2>
auto
min(T1&& a, T2&& b) {
    return a < b ? std::forward<T1>(a) : std::forward<T2>(b);
}

template<class T, class... Args>
auto
min(T&& a, Args&&... args) {
    return min(std::forward<T>(a), min(std::forward<Args>(args)...));
}

/// @brief 基于迭代器的范围比较模板函数
/// @param first 第一个迭代器(包含)
/// @param last 最后一个迭代器(不包含)
/// @param comp 比较函数
/// @return 范围内按照比较函数比较后的最大值
template<class Iterator, class Compare>
auto
range_compare(Iterator first, Iterator last, Compare comp = Compare()) ->
    typename std::iterator_traits<Iterator>::value_type {
    if (first == last) {
        throw std::invalid_argument("Range cannot be empty");
    }
    auto maxValue = *first;
    for (Iterator it = std::next(first); it != last; ++it) {
        if (comp(maxValue, *it)) {
            maxValue = *it;
        }
    }
    return maxValue;
}

/// @brief 基于迭代器的范围最大值模板函数
/// @param first 第一个迭代器(包含)
/// @param last 最后一个迭代器(不包含)
/// @return 范围内的最大值
template<class Iterator>
auto
range_max(Iterator first, Iterator last) {
    return range_compare(first, last, std::less<typename std::iterator_traits<Iterator>::value_type>());
}

/// @brief 基于迭代器的范围最小值模板函数
/// @param first 第一个迭代器(包含)
/// @param last 最后一个迭代器(不包含)
/// @return 范围内的最小值
template<class Iterator>
auto
range_min(Iterator first, Iterator last) {
    return range_compare(first, last, std::greater<typename std::iterator_traits<Iterator>::value_type>());
}

/// 1. 通用单例模式
/// 推荐使用继承方式 class Foo : public Singleton<Foo>
/// 此时如果有 auto f1 = Singleton<Foo>::GetInstance();   则编译器会报错阻止拷贝构造，保证单例对象的唯一性
///
/// 2. 对于不继承直接使用单例模式的方式Singleton<Foo2>::GetInstance().printAddress();
/// 这种方式没有阻止单例的拷贝或赋值, 不推荐使用, 会导致单例失效 例如: auto f2 = Singleton<Foo2>::GetInstance();
/// 此时f2是一个新的实例，对Singleton<Foo2>::GetInstance()的单例进行了拷贝
template<typename T>
class Singleton { // 泛型单例
public:
    // 获取单例实例对象
    static T&
    GetInstance() {
        // 利用局部静态变量实现单例
        static T instance;
        return instance;
    }

    // 打印单例的地址
    void
    printAddress() {
        std::cout << this << std::endl;
    }

    // 禁止外部拷贝或赋值
    Singleton(const Singleton&) = delete;
    Singleton(Singleton&&)      = delete;
    Singleton&
    operator=(const Singleton&)
        = delete;
    Singleton&
    operator=(Singleton&&)
        = delete;

protected:
    // 禁止外部构造和析构
    Singleton()  = default;
    ~Singleton() = default;
};

/// @brief 运行时断言函数, 若condition为false, 则抛出异常
/// @param condition 断言条件
/// @param what 异常信息
inline void
assure(bool condition, std::string_view what = "Assertion failed!") {
    if (!condition) {
        throw std::runtime_error{what.data()};
    }
}

/// @brief 随机数生成器
/// @tparam T 数据类型
/// @param begin 开始值(包含)
/// @param end 结束值(不包含)
template<class T>
T
getRandom(T begin, T end) {
    assert(begin <= end);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<T> dis(begin, end);
    return dis(gen);
}

/// @brief 获取当前时间戳
/// @return 当前时间戳
/// @note 单位为毫秒
inline int64_t
getCurrentTimeStamp() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

// 获取当前时间的格式化字符串
// format 时间格式字符串, 默认为"%Y-%m-%d %H:%M:%S"
inline string
getCurrentTime(const string& format = "%Y-%m-%d %H:%M:%S") {
    time_t now = time(nullptr);
    char buf[1024];
    strftime(buf, sizeof(buf), format.c_str(), localtime(&now));
    return buf;
}

/// @brief 计时器类声明
class stopwatch {
    using system_clock = std::chrono::system_clock;
    using time_point   = system_clock::time_point;

protected:
    bool started{false};   // 是否已经开始计时
    bool stopped{false};   // 是否已经停止计时
    double rate{1.f};      // 时间比例(默认为1us)
    time_point begin_time; // 开始时间
    time_point end_time;   // 停止时间
    size_t tick{0ull};     // duration的tick数(纳秒)

public:
    enum class TimeType {
        ns = 1,
        us = 1000,
        ms = 1000000,
        s  = 1000000000
    };

    // 指定rate倍数的ns作为单位
    stopwatch(double rate = 1.0)
        : started(false),
          stopped(false),
          rate(rate),
          tick(0),
          begin_time(system_clock::now()),
          end_time(system_clock::now()) {}

    // 指定时间类型作为单位
    stopwatch(TimeType type)
        : stopwatch(static_cast<double>(type)) {}

    virtual ~stopwatch() = default;

    inline void
    start() {
        reset();
        started    = true;
        stopped    = false;
        begin_time = system_clock::now();
    }

    inline void
    stop() {
        if (!started) {
            return;
        }
        stopped  = true;
        end_time = system_clock::now();
        tick = static_cast<size_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - begin_time).count());
    }

    inline void
    reset() {
        started = false;
        stopped = false;
        tick    = 0;
    }

    inline double
    duration() {
        // 未启动计时器, 抛出异常
        if (!started) {
            throw std::runtime_error("StopWatch::duration(): Not started yet.");
            return -1;
        }
        // 未停止计时器, 停止计时器计算duration后再启动
        if (!stopped) {
            stop();
            stopped = true;
        }
        return static_cast<double>(tick) / rate; // 返回单位为rate倍的ns
    }
};                                               // class StopWatch

// 对TimeType的输出运算符重载
inline std::ostream&
operator<<(std::ostream& os, stopwatch::TimeType type) {
    switch (type) {
        case stopwatch::TimeType::ns :
            os << "ns";
            break;
        case stopwatch::TimeType::us :
            os << "us";
            break;
        case stopwatch::TimeType::ms :
            os << "ms";
            break;
        case stopwatch::TimeType::s :
            os << "s";
            break;
        default :
            break;
    }
    return os;
}

class auto_stopwatch : public stopwatch {
public:
    inline auto_stopwatch(double rate = 1.0)
        : stopwatch(rate) {
        this->start();
    }

    inline auto_stopwatch(TimeType type)
        : stopwatch(type) {
        this->start();
    }

    inline ~auto_stopwatch() {
        std::cout << "duration time: " << this->duration() << static_cast<TimeType>(rate) << std::endl;
    }
}; // class auto_stopWatch

// 辅助函数, 将单个参数转化为字符串
template<typename T>
string
to_string(T&& arg) {
    std::stringstream oss;
    oss << std::forward<T>(arg);
    return oss.str();
}

// 使用模板折叠格式化日志消息，支持 "{}" 占位符
template<typename... Args>
string
FormatMessage(const string& fmt, Args&&... args) {
    vector<string> argStr = {to_string(std::forward<Args>(args))...};
    std::ostringstream oss;

    size_t argIndex    = 0;
    size_t pos         = 0;
    size_t placeholder = fmt.find("{}", pos);

    while (placeholder != string::npos) {
        oss << fmt.substr(pos, placeholder - pos);
        if (argIndex < argStr.size()) {
            oss << argStr[argIndex++];
        } else {
            // 没有足够的参数，保留 "{}"
            oss << "{}";
        }
        pos         = placeholder + 2; // 跳过 "{}"
        placeholder = fmt.find("{}", pos);
    }

    // 添加剩余的字符串
    oss << fmt.substr(pos);

    // 如果还有剩余的参数，按原方式拼接
    while (argIndex < argStr.size()) {
        oss << argStr[argIndex++];
    }

    return oss.str();
}

class AsyncLogSystem : public Singleton<AsyncLogSystem> {
    friend class Singleton<AsyncLogSystem>;

public:
    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
    };

private:
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
    LogQueue _logQue;              // 日志队列
    std::thread _worker;           // 工作线程
    std::ofstream _logFile;        // 日志输出文件
    std::ostream& _console;        // 日志输出流
    std::atomic<bool> _isShutDown; // 是否关闭
};                                 // class AsyncLogSystem

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
                std::cout << "join thread " << thread.get_id() << std::endl;
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

class Defer {
public:
    Defer(std::function<void()> func)
        : _func(func) {}

    ~Defer() {
        _func();
    }

private:
    std::function<void()> _func;
}; // class Defer

// 线程守卫类，用于确保线程在离开作用域时被正确地加入
class thread_guard {
public:
    explicit thread_guard(std::thread& t)
        : _t(t) {}

    ~thread_guard() {
        if (_t.joinable()) {
            _t.join();
        }
    }

public:
    thread_guard(const thread_guard&) = delete;
    thread_guard&
    operator=(const thread_guard&)
        = delete;

private:
    std::thread& _t;
}; // class thread_guard

} // namespace lyf
