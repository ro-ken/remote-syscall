#ifndef _RSC_H
#define _RSC_H  1

/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

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

/* 消息元素长度定义 */
#define RSC_HEADER_COUNT sizeof(struct rsc_header)
#define RSC_HEADER_RETURN_COUNT sizeof(struct rsc_header_return)
#define POINT_HEADER_COUNT sizeof(struct point_header)
#define RSC_REGS_COUNT sizeof(struct rsc_regs)

/* 系统调用重定向到一个不存在的系统调用号 */
#define RSC_REDIRECT_SYSCALL 10000

/*
远程系统调用请求格式:
----------------------
|  struct rsc_header  |
----------------------
|  struct rsc_regs  |
---------------------
|   point_header    |
---------------------
        ...
---------------------
|   point_header    |
---------------------
|                   |
|       buffer      |
|                   |
---------------------

远程系统调用返回格式:
-------------------------------
|  struct rsc_return_header  |
-------------------------------
|   point_header             |
------------------------------
        ...
------------------------------
|   point_header             |
------------------------------
|                            |
|       buffer               |
|                            |
------------------------------
*/

// 远程系统调用请求头 
struct rsc_header {
    unsigned long long int syscall;     // 系统调用号
    unsigned int p_number;              // 指针参数个数
    unsigned int count;					// 远程系统调用请求长度, 字节为单位
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

// 系统调用指针头
struct point_header{
    unsigned int p_location; // 指针指向的是第几个参数（根据函数原型声明从左到右）
    unsigned int p_length;  // 指针指向的内存的长度
};

// 远程系统调用执行结果头 
struct rsc_header_return {
    unsigned long long int rax;         // 返回值
    unsigned int p_number;              // 指针参数个数
    unsigned int errno;                 // 全局变量 errno
};

// 程序出现严重错误，直接结束程序
#define FATAL(...) \
    do { \
        fprintf(stderr, "[Remote syscall]: " __VA_ARGS__); \
        fputc('\n', stderr); \
        printf("[Perror]: %s", strerror(errno));\
        exit(EXIT_FAILURE); \
    } while (0)

/* 系统调用指针表 */
void syscall_point_query(char * syscall_request);

// 远程系统调用请求编组
void syscall_request_encode();

// 远程系统调用请求解组
void syscall_request_decode();

// 远程系统调用执行结果编组
void syscall_return_encode();

// 远程系统调用执行结果解组
void syscall_return_decode();

#endif