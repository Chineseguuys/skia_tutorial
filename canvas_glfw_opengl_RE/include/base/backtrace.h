#ifndef _BACK_TRACE_H_
#define _BACK_TRACE_H_

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cxxabi.h>  // 用于 demangle C++ 符号

namespace bt {

static const int max_frames = 32;

void print_stacktrace(int skip = 1) {
    void* addrlist[max_frames + 1];
    
    // 获取当前线程的调用堆栈
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));
    
    if (addrlen == 0) {
        fprintf(stderr, "  <empty, possibly corrupt>\n");
        return;
    }
    
    // 将地址转换为符号信息
    char** symbollist = backtrace_symbols(addrlist, addrlen);
    
    // 分配字符串用于存储 demangled 符号名
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);
    
    // 跳过当前函数（print_stacktrace）本身
    for (int i = skip; i < addrlen; i++) {
        char* begin_name = nullptr;
        char* begin_offset = nullptr;
        char* end_offset = nullptr;
        
        // 找到函数名和偏移量的开始和结束位置
        for (char* p = symbollist[i]; *p; ++p) {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }
        
        if (begin_name && begin_offset && end_offset && 
            begin_name < begin_offset) {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';
            
            // Demangle C++ 符号名
            int status;
            char* ret = abi::__cxa_demangle(begin_name, funcname, 
                                           &funcnamesize, &status);
            if (status == 0) {
                funcname = ret;  // 使用 demangled 名称
                fprintf(stderr, "  %s : %s+%s\n", 
                        symbollist[i], funcname, begin_offset);
            } else {
                // Demangle 失败，使用原始名称
                fprintf(stderr, "  %s : %s+%s\n", 
                        symbollist[i], begin_name, begin_offset);
            }
        } else {
            // 无法解析，直接输出原始行
            fprintf(stderr, "  %s\n", symbollist[i]);
        }
    }
    
    free(funcname);
    free(symbollist);
}

// 包装函数，方便使用
class StackTracer {
public:
    StackTracer(const char* msg = nullptr) : message(msg) {
        if (message) {
            fprintf(stderr, "\n=== Stack Trace (%s) ===\n", message);
        } else {
            fprintf(stderr, "\n=== Stack Trace ===\n");
        }
        print_stacktrace(2);  // 跳过 StackTracer 构造函数和调用者
    }
    
    ~StackTracer() {
        fprintf(stderr, "=== End Stack Trace ===\n\n");
    }
    
private:
    const char* message;
};

// 使用宏简化调用
#define TRACE_STACK(msg) StackTracer __stack_tracer__(msg)

} // namespace backtrace

#endif // _BACK_TRACE_H_