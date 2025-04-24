#pragma once

#include <iostream>

// 用于快捷打印变量名和值的宏
#define m_print(arg) std::cout << #arg << " = " << arg << std::endl
// 快捷输出并附带文件行号定位
#define m_debug(...) std::cout << "[" << __FILE__ << ":" << __LINE__ << "]: " << __VA_ARGS__ << std::endl

namespace lyf {
using std::cout, std::endl, std::string;

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

} // namespace lyf
