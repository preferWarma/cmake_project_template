#include "lyf/TypeTool.h"
#include <gtest/gtest.h>
#include <string>

// 测试 getTypeName 函数
TEST(TypeToolTest, GetTypeNameTest) {
    int intVar           = 42;
    std::string typeName = lyf::getTypeName<decltype(intVar)>();
    EXPECT_EQ(typeName, "int");
}

// 测试 type_class::get 函数
TEST(TypeToolTest, TypeClassGetTest) {
    double doubleVar          = 3.14;
    std::string wholeTypeName = lyf::type_class<decltype(doubleVar)>::get();
    EXPECT_EQ(wholeTypeName, "double");
}

// 测试 typeof 宏
TEST(TypeToolTest, TypeofMacroTest) {
    std::string stringVar = "Hello, World!";
    std::string typeName  = typeof(stringVar);
    EXPECT_EQ(typeName, "std::string");
}

// 测试 getWholeTypeName 宏
TEST(TypeToolTest, GetWholeTypeNameMacroTest) {
    bool boolVar              = true;
    std::string wholeTypeName = getWholeTypeName(boolVar);
    EXPECT_EQ(wholeTypeName, "bool");
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
