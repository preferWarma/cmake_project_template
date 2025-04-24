#pragma once

#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace lyf {

using std::string, std::vector;

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

} // namespace lyf
