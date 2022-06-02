#ifndef _RSC_H
#define _RSC_H  0

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

// struct rsc_header size
#define RSC_HEADER_SIZE     sizeof(struct rsc_header)

// redirect syscall
#define RSC_REDIRECT_SYSCALL 10000

// syscall classify 
#define NO_POINTER              0  // 不带指针参数的系统调用
#define IN_POINTER              1  // 带输入指针参数的系统调用
#define OUT_POINTER             2  // 带输出指针参数的系统调用
#define IO_POINTER              3  // 带输入输出指针参数的系统调用
#define IN_SEQUENCE_POINTER     4  // 带连续输入指针参数的系统调用, 如 sys_poll, 使用结构体数据传递数据

// fatal error, exit process
#define FATAL(...) \
    do { \
        fprintf(stderr, ": " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)


// syscall header
struct rsc_header {
    unsigned long long int syscall;     // 系统调用号
    unsigned int p_flag;              	// 系统调用分类
    unsigned int size;					// 远程系统调用请求长度, 字节为单位
    unsigned int error;                 // 出错信息

    // syscall register parameters
    unsigned long long int rax;
    unsigned long long int rdi;
    unsigned long long int rsi;
    unsigned long long int rdx;
    unsigned long long int r10;
    unsigned long long int r8;
    unsigned long long int r9;

    // describe information of pointer parameters
    unsigned int p_location_in;     // 输入指针所在位置
    unsigned int p_location_out;    // 输出指针所在位置
    unsigned int p_count_in;        // 输入指针需要操作的字节数
    unsigned int p_count_out;       // 输出指针需要操作的字节数

    // memory  synchronize (only server use)
    char * p_addr_in;               // 输入指针指向的内存, 用于服务端进行指针重定向
    char * p_addr_out;              // 输出指针指向的内存, 用于服务端进行指针重定向
};

/* syscall bitmap, manage implemented remote system calls */
#define LLSIZE (sizeof(unsigned long long int) * 8)
#define SET_MASK 0x0000000000000001
#define ISSET_MASK 0xfffffffffffffffe
#define RESET_MASK 0xfffffffffffffffe

#endif