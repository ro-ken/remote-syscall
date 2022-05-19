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


#define RSC_MAX_ERRNO_SIZE 1000
#define NR_GETPID 39
#define RSC_REDIRECT_SYSCALL 10000

/* 系统调用参数传递 */
struct syscall_para {
    unsigned long long int rax;
    unsigned long long int rdi;
    unsigned long long int rsi;
    unsigned long long int rdx;
    unsigned long long int rcx;
    unsigned long long int r8;
    unsigned long long int r9;
};

/* 系统调用结果返回 */
struct syscall_return {
    unsigned long long int rax;
    int errno_num;
    char errno_info[RSC_MAX_ERRNO_SIZE];
};

#define FATAL(...) \
    do { \
        fprintf(stderr, "RSC: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)


#endif