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

// 重定向系统调用
#define RSC_REDIRECT_SYSCALL 10000

// 系统调用分类 
#define NO_POINTER              0  // 不带指针参数的系统调用
#define IN_POINTER              1  // 带输入指针参数的系统调用
#define OUT_POINTER             2  // 带输出指针参数的系统调用
#define IO_POINTER              3  // 带输入输出指针参数的系统调用
#define IN_SEQUENCE_POINTER     4  // 带连续输入指针参数的系统调用, 如 sys_poll, 使用结构体数据传递数据

// 程序出现严重错误，直接结束程序
#define FATAL(...) \
    do { \
        fprintf(stderr, "[%s][%s]: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

// 函数出现问题, 解释原因并退出
#define ERROR(...) \
    do { \
        fprintf(stderr, "[%s][%s]: " __VA_ARGS__); \
        fprintf(stderr, "errno, %d, strerror: %s\n", errno, strerror(errno)); \
        fputc('\n', stderr); \
        return -1; \
    } while (0)

// 系统调用请求头 
struct rsc_header {
    unsigned long long int syscall;     // 系统调用号
    unsigned int p_flag;              	// 系统调用分类
    unsigned int size;					// 远程系统调用请求长度, 字节为单位
    unsigned int error;                 // 出错信息

    // 系统调用传参寄存器
    unsigned long long int rax;
    unsigned long long int rdi;
    unsigned long long int rsi;
    unsigned long long int rdx;
    unsigned long long int r10;
    unsigned long long int r8;
    unsigned long long int r9;

    // 系统调用指针参数描述信息
    unsigned int p_location_in;     // 输入指针所在位置
    unsigned int p_location_out;    // 输出指针所在位置
    unsigned int p_count_in;        // 输入指针需要操作的字节数
    unsigned int p_count_out;       // 输出指针需要操作的字节数

    char * p_addr_in;               // 输入指针指向的内存, 用于服务端进行指针重定向
    char * p_addr_out;              // 输出指针指向的内存, 用于服务端进行指针重定向
};

/* 系统调用位示图, 用来管理已实现的系统调用(已实现置位1, 否则置位0) */
#define LLSIZE (sizeof(unsigned long long int) * 8)
#define SET_MASK 0x0000000000000001
#define ISSET_MASK 0xfffffffffffffffe
#define RESET_MASK 0xfffffffffffffffe

// 置位系统调用号对应的位示图bit位
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

// 查询当前系统调用是否已实现
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

// 置位系统调用号对应的位示图bit位
void reset_bitmap(unsigned long long int * syscall_bitmap, unsigned int syscall){
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