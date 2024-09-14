#include <iostream>
#include <cstdlib>
#include <cassert>
#include <cstddef>
#include <new>
#include <unordered_map>
#include <fstream>
#include "memCheck.h"

using std::size_t;
using std::cout, std::endl;
using std::unordered_map;
using std::string;

#undef new  // 取消new宏定义

bool __traceFlag = true;
bool __activeFlag = false;
std::ofstream __fileStream;
memCheck::LogType __logType = memCheck::LogType::Just_Cout;

#define MASK 0x1234'0000'0000'0000

namespace {
    struct OtherInfo {
        const char* file;
        unsigned long line;
    };

    struct ListNodeBase { // 双链表节点
        ListNodeBase* prev{ this };
        ListNodeBase* next{ this };
    };

    struct ListNode : ListNodeBase { // 存储信息的节点
        size_t size{};
        OtherInfo info;
    };

    ListNodeBase dummyHead; // 虚拟头节点

    struct Sentinel {   // 用于析构时检查内存泄漏
        ~Sentinel() {
            auto next = dummyHead.next; // 没有内存泄漏的话, next应该指向自己
            bool hasLeak = (next != &dummyHead);

            if (hasLeak) {
                if (__logType != memCheck::LogType::Just_Cout) {
                    __fileStream << "Leaked memory at: " << endl;
                }
                if (__logType != memCheck::LogType::Just_File) {
                    cout << "Leaked memory at: " << endl;
                }
                while (next != &dummyHead) {
                    auto node = static_cast<ListNode*>(next);
                    if (__logType != memCheck::LogType::Just_Cout) {
                        __fileStream << "\t" << node << " with size: " << (node->size ^ MASK)
                            << " (file: " << node->info.file << ", line: " << node->info.line << ")\n";
                    }
                    if (__logType != memCheck::LogType::Just_File) {
                        cout << "\t" << node << " with size: " << (node->size ^ MASK)
                            << " (file: " << node->info.file << ", line: " << node->info.line << ")\n";
                    }
                    next = node->next;
                }
            }
            else {
                if (__logType != memCheck::LogType::Just_Cout) {
                    __fileStream << "No user memory leaks!" << endl;
                }
                if (__logType != memCheck::LogType::Just_File) {
                    cout << "No user memory leaks!" << endl;
                }
            }
        }
    };

    Sentinel sentinel;
}   // namespace

// 开始重载new和delete
void* operator new(std::size_t size, const char* file, unsigned long line) {
    const size_t allSize = size + sizeof(ListNode);
    void* ptr = std::malloc(allSize);
    if (ptr == nullptr) {
        throw std::bad_alloc();
    }
    if (__activeFlag) {
        auto node = static_cast<ListNode*>(ptr);
        static_assert(sizeof(void*) == 8, "not 64-bit system");
        node->size = size | MASK;
        node->info.file = file;
        node->info.line = line;
        // 插入到链表中
        node->prev = &dummyHead;
        node->next = dummyHead.next;
        dummyHead.next->prev = node;
        dummyHead.next = node;
    }
    // 返回用户可见的内存地址
    void* ret_ptr = static_cast<void*>(static_cast<unsigned char*>(ptr) + sizeof(ListNode));
    if (__traceFlag) {
        if (__logType != memCheck::LogType::Just_Cout) {
            __fileStream << "Allocated " << size << " bytes at adress: " << ret_ptr << " (file: " << file << "  line: " << line << ")" << endl;
        }
        if (__logType != memCheck::LogType::Just_File) {
            cout << "Allocated " << size << " bytes at adress: " << ret_ptr << " (file: " << file << "  line: " << line << ")" << endl;
        }
    }
    return ret_ptr;
}

void* operator new[](std::size_t size, const char* file, unsigned long line) {
    return operator new(size, file, line);
}

void operator delete(void* ptr) noexcept {
    // ptr指向的是用户可见的内存, 原始分配的内存地址是ptr - sizeof(ListNode)
    auto node = static_cast<ListNode*>(static_cast<void*>(static_cast<unsigned char*>(ptr) - sizeof(ListNode)));
    if ((node->size & MASK) == MASK) { // 是我们重载的new分配的内存
        // 从链表中删除
        node->prev->next = node->next;
        node->next->prev = node->prev;
        // 输出信息
        if (__traceFlag) {
            if (__logType != memCheck::LogType::Just_Cout) {
                __fileStream << "Deallocated memory at adress: " << ptr << endl;
            }
            if (__logType != memCheck::LogType::Just_File) {
                cout << "Deallocated memory at adress: " << ptr << endl;
            }
        }
        std::free(node);
    }
    else {  // 不是我们重载的new分配的内存或者重复释放
        std::free(ptr); // 直接释放, 报错信息由free函数自己处理
    }
}

void operator delete[](void* ptr) noexcept {
    operator delete(ptr);
}

void operator delete(void* ptr, std::size_t size) noexcept {
    operator delete(ptr);
}

void operator delete[](void* ptr, std::size_t size) noexcept {
    operator delete[](ptr);
}