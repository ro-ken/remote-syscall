#ifndef _RSC_H
#define _RSC_H  1

/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>



#define RSC_SERVER_IP "127.0.0.1"
#define RSC_SERVER_PORT 10000
#define RSC_MAX_CONNECTS 5

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
    char error_info[1000];
};

#define FATAL(...) \
    do { \
        fprintf(stderr, "RSC: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

#endif