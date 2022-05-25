#ifndef _RSC_CLIENT_H
#define _RSC_CLIENT_H  2

#include "include/rsc_include.h"

/* 
 * 功能：远程系统调用请求解组
 * 输入：
    [-] sockfd_s: socket文件描述符
    [-] regs    : struct rsc_regs, 用来存放系统调用请求的参数
    [-] syscall : 获取系统调用号
    [-] p_flag  : 获取系统调用类别
 * 返回：函数执行是否成狗
*/
int syscall_request_decode(int sockfd_s, struct rsc_regs * regs, struct pointer_manager * p_manager, unsigned int * syscall)

// 远程系统调用请求解组辅助函数
int no_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager);
int input_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);
int out_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);
int io_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);

// 远程系统调用请求执行
int syscall_execute(struct rsc_regs * regs, struct rsc_return_header * return_header);

/* 远程系统调用请求执行结果编组 */
int syscall_return_encode(int sockfd_s, struct rsc_regs * regs, struct pointer_manager * p_manager);

// 远程系统调用请求执行结果编组辅助函数
int no_pointer_server_encode(struct rsc_regs * regs, struct pointer_manager * p_manager);
int input_pointer_server_encode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);
int out_pointer_server_encode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);
int io_pointer_server_encode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);


#endif