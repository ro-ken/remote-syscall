#ifndef _RSC_INCLUDE_H
#define _RSC_INCLUDE_H  0

/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

/* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

// 宏定义远程系统调用传递的数据结构长度 
#define RSC_HEADER_SIZE     sizeof(struct rsc_header)
#define RSC_REGS_SIZE       sizeof(struct rsc_regs)
#define RSC_POINTER_SIZE    sizeof(struct rsc_pointer)
#define RSC_RETURN_HEADER   sizeof(struct rsc_return_header)
#define RSC_IO_POINTER_SIZE sizeof(struct rsc_io_pointer)


// 系统调用请求头 
struct rsc_header {
    unsigned long long int syscall;     // 系统调用号
    unsigned int p_flag;              	// 系统调用分类
    unsigned int size;					// 远程系统调用请求长度, 字节为单位
};

// 系统调用寄存器参数
struct rsc_regs {
    unsigned long long int rax;
    unsigned long long int rdi;
    unsigned long long int rsi;
    unsigned long long int rdx;
    unsigned long long int r10;
    unsigned long long int r8;
    unsigned long long int r9;
};

// 指针参数描述信息
struct rsc_pointer {
    unsigned int p_location;        // 指针参数是系统调用中的第几个参数(从左到右，从1开始计算)
    unsigned int p_count;           // 指针参数要操作的字节数，比如 sys_read 读 count 个字节到 buf 指向的内存中
}

// 远程系统调用执行结果头 
struct rsc_return_header {
    unsigned long long int syscall;     // 系统调用号
    unsigned int p_flag;                // 系统调用类型
    unsigned int size;                  // 远程系统调用请求长度, 字节为单位
    unsigned int error;                 // 全局变量 errno
    unsigned int error_size;            // 出错信息长度(如果有的话)
};

// 程序出现严重错误，直接结束程序
#define FATAL(...) \
    do { \
        fprintf(stderr, "[Remote syscall]: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

/* 系统调用分类 */
#define NO_POINTER 0                // 不带指针参数的系统调用
#define INPUT_POINTER 1             // 带输入指针参数的系统调用
#define OUTPUT_POINTER 2            // 带输出指针参数的系统调用
#define IO_POINTER 3                // 带输入输出指针参数的系统调用
#define INPUT_SEQUENCE_POINTER 4    // 带连续输入指针参数的系统调用, 如 sys_poll, 使用结构体数据传递数据

// 带输入输出的系统调用的指针参数描述信息
struct rsc_io_pointer {
    unsigned int p_location_in;
    unsigned int p_count_in;
    unsigned long long int addr_in;

    unsigned int p_location_out;
    unsigned int p_count_out;
};

// 指针管理器, 记录远程系统调用请求的指针参数信息以及server为其开辟的缓冲区地址
struct pointer_manager {
    unsigned int p_location_in;
    unsigned int p_location_out;

    unsigned int p_count_in;
    unsigned int p_count_out;

    unsigned int p_flag;

    char * p_addr_in;
    char * p_addr_out;
}

// 系统调用位示图
#define LLSIZE (sizeof(unsigned long long int) * 8)
#define SET_MASK 0x0000000000000001
#define ISSET_MASK 0xfffffffffffffffe
#define RESET_MASK 0xfffffffffffffffe

void set_bitmap(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return;
    }
    int base = syscall / LLSIZE;    // 获取基址
    int surplus = syscall % LLSIZE; // 余值

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    *base_p = base_n | ((( base_n >> surplus) | SET_MASK) << surplus);
}

int is_set(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return -1;
    }
    int base = syscall / LLSIZE;    // 获取基址
    int surplus = syscall % LLSIZE; // 余值

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    base_n = (base_n >> surplus) | ISSET_MASK;
    return base_n==0xffffffffffffffff?1:-1;
}

void reset_bitmap(){
    if (syscall < 0 && syscall > 547){
        return;
    }
    int base = syscall / LLSIZE;    // 获取基址
    int surplus = syscall % LLSIZE; // 余值

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    *base_p = base_n | ((( base_n >> surplus) & RESET_MASK) << surplus);
}

#endif