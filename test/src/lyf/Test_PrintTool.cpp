#include "lyf/PrintTool.h"
#include <gtest/gtest.h>
#include <sstream>
#include <vector>

// 测试 m_print 宏
TEST(PrintToolTest, TestMPrint) {
    int num = 42;
    std::ostringstream oss;
    auto old_cout_buf = std::cout.rdbuf(oss.rdbuf());
    m_print(num);
    std::cout.rdbuf(old_cout_buf);
    EXPECT_EQ(oss.str(), "num = 42\n");
}

// 测试 m_debug 宏
TEST(PrintToolTest, TestMDebug) {
    std::ostringstream oss;
    auto old_cout_buf = std::cout.rdbuf(oss.rdbuf());
    m_debug("Test message");
    std::cout.rdbuf(old_cout_buf);
    std::string expected = "[" __FILE__ ":" + std::to_string(__LINE__ - 2) + "]: Test message\n";
    EXPECT_EQ(oss.str(), expected);
}

// 测试 print_container 模板函数（迭代器方式）
TEST(PrintToolTest, TestPrintContainerIterator) {
    std::vector<int> vec = {1, 2, 3};
    std::ostringstream oss;
    lyf::print_container(vec, " ", oss);
    EXPECT_EQ(oss.str(), "1 2 3 \n");
}

// 测试 print_container 模板函数（范围方式）
TEST(PrintToolTest, TestPrintContainerRange) {
    std::vector<int> vec = {1, 2, 3};
    std::ostringstream oss;
    lyf::print_container(vec.begin(), vec.end(), " ", oss);
    EXPECT_EQ(oss.str(), "1 2 3 \n");
}

// 测试 blue 函数
TEST(PrintToolTest, TestBlue) {
    std::string str      = "Test";
    std::string expected = "\033[34m" + str + "\033[0m";
    EXPECT_EQ(lyf::blue(str), expected);
}

// 测试 green 函数
TEST(PrintToolTest, TestGreen) {
    std::string str      = "Test";
    std::string expected = "\033[32m" + str + "\033[0m";
    EXPECT_EQ(lyf::green(str), expected);
}

// 测试 red 函数
TEST(PrintToolTest, TestRed) {
    std::string str      = "Test";
    std::string expected = "\033[31m" + str + "\033[0m";
    EXPECT_EQ(lyf::red(str), expected);
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
