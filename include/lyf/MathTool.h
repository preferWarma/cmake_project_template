#pragma once

#include <iterator>
#include <random>
#include <stdexcept>
#include <utility>

namespace lyf {

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

/// @brief 随机数生成器
/// @tparam T 数据类型
/// @param begin 开始值(包含)
/// @param end 结束值(不包含)
template<class T>
T
getRandom(T begin, T end) {
    if (begin >= end) {
        throw std::invalid_argument("Begin must be less than end");
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<T> dis(begin, end);
    return dis(gen);
}

} // namespace lyf
