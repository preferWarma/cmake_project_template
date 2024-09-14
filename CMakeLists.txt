# CMakeLists.txt
cmake_minimum_required(VERSION 3.20) # cmake的最低版本要求

project(code_template_cmake CXX) # 项目名称 CXX表示C++项目

set(CMAKE_CXX_STANDARD_REQUIRED TRUE) # C++标准要求
set(CMAKE_CXX_STANDARD 20) # 设置C++标准版本

add_subdirectory(src) # 添加src文件夹作为子目录

set(exeFileName main) # 设置可执行文件名称

add_executable(${exeFileName} main.cpp) # 添加可执行文件

target_link_libraries(${exeFileName} PUBLIC MyLib) # 添加链接库, MyLib为src文件夹下CMakeLists.txt中定义的MyLib

# 编译选项
target_compile_options(${exeFileName} PUBLIC
    -Wall
    -Wextra
    -Werror
    -Wno-unknown-warning-option
    -Wno-int-to-pointer-cast
    -Wno-class-memaccess
    -Wno-assume
)

# 以下是添加测试文件的使用(方便单文件测试运行)
add_executable(test test.cpp)
target_link_libraries(test PUBLIC MyLib)