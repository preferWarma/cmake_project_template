#include "lyf/StringTool.h"
#include <gtest/gtest.h>

using namespace lyf;

// 测试 split 函数（分隔符为 char）
TEST(StringToolTest, SplitChar) {
    std::string str                 = "hello,world";
    char delim                      = ',';
    std::vector<std::string> result = split(str, delim);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "hello");
    EXPECT_EQ(result[1], "world");
}

// 测试 split 函数（分隔符为 string）
TEST(StringToolTest, SplitString) {
    std::string str                 = "hello||world";
    std::string delim               = "||";
    std::vector<std::string> result = split(str, delim);
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "hello");
    EXPECT_EQ(result[1], "world");
}

// 测试 regex_match 函数
TEST(StringToolTest, RegexMatch) {
    std::string str                 = "hello 123 world";
    std::string pattern             = "\\d+";
    std::vector<std::string> result = regex_match(str, pattern);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "123");
}

// 测试 replace_first 函数
TEST(StringToolTest, ReplaceFirst) {
    std::string str       = "hello world";
    std::string old_value = "hello";
    std::string new_value = "hi";
    std::string result    = replace_first(str, old_value, new_value);
    EXPECT_EQ(result, "hi world");
}

// 测试 replace_all 函数
TEST(StringToolTest, ReplaceAll) {
    std::string str       = "hello hello";
    std::string old_value = "hello";
    std::string new_value = "hi";
    std::string result    = replace_all(str, old_value, new_value);
    EXPECT_EQ(result, "hi hi");
}

// 测试 replace_last 函数
TEST(StringToolTest, ReplaceLast) {
    std::string str       = "hello hello";
    std::string old_value = "hello";
    std::string new_value = "hi";
    std::string result    = replace_last(str, old_value, new_value);
    EXPECT_EQ(result, "hello hi");
}

// 测试 begin_with 函数
TEST(StringToolTest, BeginWith) {
    std::string str    = "hello world";
    std::string prefix = "hello";
    EXPECT_TRUE(begin_with(str, prefix));
}

// 测试 end_with 函数
TEST(StringToolTest, EndWith) {
    std::string str    = "hello world";
    std::string suffix = "world";
    EXPECT_TRUE(end_with(str, suffix));
}

// 测试 FormatMessage 函数
TEST(StringToolTest, FormatMessage) {
    std::string fmt    = "Hello, {}! You are {} years old.";
    std::string name   = "John";
    int age            = 30;
    std::string result = FormatMessage(fmt, name, age);
    EXPECT_EQ(result, "Hello, John! You are 30 years old.");
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
