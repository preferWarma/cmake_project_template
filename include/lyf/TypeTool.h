#pragma once

#include <cxxabi.h>
#include <string>

// 用于获取变量类型的快捷调用宏, 此方法会忽略掉没有显示指明的默认参数类型,
// 例如template<class T, class U = string> class A {};则对于auto a = A<Color>();将返回A<Color>而不是A<Color, string>,
// 若要获取完整的类型, 可以使用getWholeTypeName(x)方法
#define typeof(x) lyf::getTypeName<decltype(x)>()

// 用于获取变量类型的快捷调用宏, 此方法会获取完整的类型, 包括默认参数类型
#define getWholeTypeName(x) lyf::type_class<decltype(x)>::get()

namespace lyf {
using std::string;

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
    return s.substr(pos1, pos2 - pos1);
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
        // 包含cuv_saver结构体的全名
        string all_realName = string{abi::__cxa_demangle(typeid(cvr_saver<T>).name(), nullptr, nullptr, nullptr)};
        auto pos1           = all_realName.find_first_of('<') + 1; // 第一个'<'后的位置
        auto pos2           = all_realName.find_last_of('>');      // 最后一个'>'的位置
        return all_realName.substr(pos1, pos2 - pos1);             // 去掉干扰信息
    }                                                              // class type_class
};

} // namespace lyf
