# 查找 GoogleTest
find_package(GTest REQUIRED)

# 包含 GoogleTest
include_directories(${GTEST_INCLUDE_DIRS})

set(LYF_SOURCES
    src/lyf/Test_TypeTool.cpp
    src/lyf/Test_Helper.cpp
    src/lyf/Test_LogSystem.cpp
    src/lyf/Test_MathTool.cpp
    src/lyf/Test_PrintTool.cpp
    src/lyf/Test_Singleton.cpp
    src/lyf/Test_Stopwatch.cpp
    src/lyf/Test_StringTool.cpp
    src/lyf/Test_ThreadPool.cpp
)

foreach(SOURCE ${LYF_SOURCES})
    get_filename_component(TEST_NAME ${SOURCE} NAME_WE) # 获取文件名
    add_executable(${TEST_NAME} ${SOURCE}) # 添加可执行文件
    target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include) # 添加头文件路径
    target_link_libraries(${TEST_NAME} ${GTEST_LIBRARIES} pthread) # 链接gtest

    # 包含测试
    include(GoogleTest)
    gtest_discover_tests(${TEST_NAME})
endforeach(SOURCE ${LYF_SOURCES})
