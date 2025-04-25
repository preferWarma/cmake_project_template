# CMake项目模板

这是一个使用CMake构建的C++项目模板，包含了多个实用的工具类和功能模块，如日志系统、线程池、字符串处理工具等，同时配备了详细的单元测试。

## 项目结构
```
cmake_project_template/
├── include/           # 头文件目录
│   ├── lyf/           # 自定义库头文件
│   │   ├── Config.h   # 配置文件
│   │   ├── Helper.h   # 辅助工具
│   │   ├── LogSystem.h # 日志系统
│   │   ├── MathTool.h # 数学工具
│   │   ├── PrintTool.h # 打印工具
│   │   ├── Singleton.h # 单例模式
│   │   ├── Stopwatch.h # 计时器
│   │   ├── StringTool.h # 字符串处理工具
│   │   ├── ThreadPool.h # 线程池
│   │   ├── TypeTool.h   # 类型工具
│   ├── lyf.h            # 统一头文件包含
├── test/              # 测试代码目录
│   ├── src/           # 测试源文件
│   │   ├── lyf/       # 自定义库测试文件
│   │   │   ├── Test_TypeTool.cpp
│   │   │   ├── Test_Helper.cpp
│   │   │   ├── Test_LogSystem.cpp
│   │   │   ├── Test_MathTool.cpp
│   │   │   ├── Test_PrintTool.cpp
│   │   │   ├── Test_Singleton.cpp
│   │   │   ├── Test_Stopwatch.cpp
│   │   │   ├── Test_StringTool.cpp
│   │   │   ├── Test_ThreadPool.cpp
│   ├── CMakeLists.txt # 测试CMake配置
├── main.cpp           # 主程序入口
├── CMakeLists.txt     # 项目CMake配置
├── .gitignore         # Git忽略文件
├── .vscode/           # VSCode配置目录
│   └── settings.json  # VSCode设置
```

## 功能模块

### 1. 日志系统 (`lyf/LogSystem.h`)
- **异步日志记录**：支持将日志异步输出到文件和控制台。
- **日志级别**：提供 `DEBUG`、`INFO`、`WARN`、`ERROR` 四个日志级别，可通过宏 `LOG_DEBUG`、`LOG_INFO`、`LOG_WARN`、`LOG_ERROR` 进行记录。
- **自定义日志**：可使用 `LOG_ONLY` 宏记录自定义格式的日志。

### 2. 线程池 (`lyf/ThreadPool.h`)
- **任务管理**：基于C++11实现，支持任务的提交和执行。
- **线程管理**：自动管理线程的生命周期，可查询空闲线程数量并停止线程池。

### 3. 字符串处理工具 (`lyf/StringTool.h`)
- **字符串分割**：支持以字符或字符串为分隔符进行分割。
- **正则匹配**：使用正则表达式匹配字符串。
- **字符串替换**：支持替换第一个、所有或最后一个指定子串。
- **字符串判断**：可判断字符串是否以指定前缀或后缀开头。
- **字符串格式化**：支持使用 `{}` 占位符进行格式化。

### 4. 辅助工具 (`lyf/Helper.h`)
- **运行时断言**：使用 `assure` 函数进行运行时断言。
- **时间处理**：提供获取当前时间和格式化时间的功能。
- **延迟执行**：支持延迟执行任务。
- **线程守卫**：确保线程安全。

### 5. 单例模式 (`lyf/Singleton.h`)
- **泛型单例**：实现了泛型单例模式，确保一个类只有一个实例，并提供全局访问点。

### 6. 计时器 (`lyf/Stopwatch.h`)
- **手动和自动计时**：支持手动开始、停止和重置计时，也可使用自动计时功能。
- **时间单位**：支持 `ns`、`us`、`ms`、`s` 四种时间单位。

### 7. 打印工具 (`lyf/PrintTool.h`)
- **变量打印**：使用 `m_print` 宏快速打印变量名和值。
- **调试信息打印**：使用 `m_debug` 宏输出调试信息并附带文件行号。
- **容器打印**：支持以迭代器方式遍历并打印容器元素。
- **颜色输出**：提供 `blue`、`green`、`red` 三种颜色的字符串输出。

### 8. 数学工具 (`lyf/MathTool.h`)
- **最大值和最小值**：支持多个参数的最大值和最小值比较。
- **范围比较**：基于迭代器的范围最大值和最小值比较。
- **随机数生成**：生成指定范围内的随机数。

### 9. 类型工具 (`lyf/TypeTool.h`)
- **类型获取**：提供 `typeof` 和 `getWholeTypeName` 宏获取变量类型。

## 编译和运行

### 编译项目
```sh
mkdir build
cd build
cmake ..
make
```

### 运行主程序
```sh
./main
```

## 单元测试

### 编译测试代码
```sh
cd build/test
make
```

### 运行测试
```sh
ctest
```

## 配置文件
在 `lyf/Config.h` 中可以配置日志文件路径和日志输出模式：
```cpp
namespace lyf {
const std::string LOG_FILE_PATH = "./log/log.txt";
const std::string LOG_MODE      = "CONSOLE";
} // namespace lyf
```

## 注意事项
- 项目使用了C++17标准的 `std::filesystem`，请确保你的编译器支持该标准。
- 如果日志目录不存在，日志系统在启动时会自动创建日志文件目录。
- 线程池在析构时会自动 `join` 所有线程。

## 贡献
如果你有任何改进建议或发现了问题，请提交Issue或Pull Request。