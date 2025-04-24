#pragma once
#include <iostream>

namespace lyf {

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

} // namespace lyf
