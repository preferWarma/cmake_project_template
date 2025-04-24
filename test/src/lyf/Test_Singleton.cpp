#include "lyf/Singleton.h"
#include <gtest/gtest.h>

// 定义一个测试类，用于测试单例模式
class TestClass {
public:
    void
    print() {
        std::cout << "TestClass print" << std::endl;
    }
};

// 测试单例模式的继承方式
TEST(SingletonTest, InheritanceMode) {
    class Foo : public lyf::Singleton<Foo> {
    public:
        void
        print() {
            std::cout << "Foo print" << std::endl;
        }
    };

    Foo& foo1 = Foo::GetInstance();
    Foo& foo2 = Foo::GetInstance();

    // 检查两个实例是否为同一个对象
    EXPECT_EQ(&foo1, &foo2);
}

// 测试单例模式的非继承方式
TEST(SingletonTest, NonInheritanceMode) {
    TestClass& test1 = lyf::Singleton<TestClass>::GetInstance();
    TestClass& test2 = lyf::Singleton<TestClass>::GetInstance();

    // 检查两个实例是否为同一个对象
    EXPECT_EQ(&test1, &test2);
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
