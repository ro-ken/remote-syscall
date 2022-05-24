#ifndef _RSC_SYSCALL_CLASSIFY_H
#define _RSC_SYSCALL_CLASSIFY_H  1

#include "rsc.h"

/* 系统调用分类 */
#define NO_POINTER 0                // 不带指针参数的系统调用
#define INPUT_POINTER 1             // 带输入指针参数的系统调用
#define OUTPUT_POINTER 2            // 带输出指针参数的系统调用
#define IO_POINTER 3                // 带输入输出指针参数的系统调用
#define INPUT_SEQUENCE_POINTER 4    // 带连续输入指针参数的系统调用, 如sys_poll, 使用结构体数据传递数据

/* 系统调用指针表 */
int syscall_pointer_handle(struct rsc_header * header, struct rsc_regs * regs, void * syscall_request);

#endif