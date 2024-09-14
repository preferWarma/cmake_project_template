#ifndef MEMCHECK_H
#define MEMCHECK_H

#include <cstddef>
#include <string>
#include <fstream>

void* operator new(std::size_t, const char*, unsigned long);
void* operator new[](std::size_t, const char*, unsigned long);

#define new new(__FILE__, __LINE__)

extern bool __traceFlag;
#define TRACE_ON() __traceFlag = true
#define TRACE_OFF() __traceFlag = false

extern bool __activeFlag;
#define MEM_ON() __activeFlag = true
#define MEM_OFF() __activeFlag = false

namespace memCheck {
    enum class LogType {
        Just_File,
        Just_Cout,
        File_And_Cout
    };
}

extern memCheck::LogType __logType;
extern std::ofstream __fileStream;

/// @brief 设置日志类型(只输出到文件, 只输出到控制台, 输出到文件和控制台)
inline void SET_LOG_TYPE(memCheck::LogType type, const std::string& fileName = "memCheck.log") {
    __logType = type;
    if (__logType == memCheck::LogType::Just_File || __logType == memCheck::LogType::File_And_Cout) {
        __fileStream.open(fileName);
    }
}

#endif  // MEMCHECK_H