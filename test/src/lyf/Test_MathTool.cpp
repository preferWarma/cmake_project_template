#include "lyf/MathTool.h"
#include <gtest/gtest.h>

// 测试 max 函数
TEST(MathToolTest, MaxFunctionTest) {
    // 测试两个不同类型的比较
    int a        = 10;
    double b     = 20.5;
    auto result1 = lyf::max(a, b);
    EXPECT_EQ(result1, b);

    // 测试多个参数的情况
    int c        = 5;
    int d        = 15;
    auto result2 = lyf::max(a, c, d);
    EXPECT_EQ(result2, d);
}

// 测试 min 函数
TEST(MathToolTest, MinFunctionTest) {
    // 测试两个不同类型的比较
    int e        = 30;
    double f     = 25.5;
    auto result3 = lyf::min(e, f);
    EXPECT_EQ(result3, f);

    // 测试多个参数的情况
    int g        = 40;
    int h        = 35;
    auto result4 = lyf::min(e, g, h);
    EXPECT_EQ(result4, e);
}

int
main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
