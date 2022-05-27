#ifndef _RSC_CLIENT_H
#define _RSC_CLIENT_H  2

#include "include/rsc_include.h"

// 建立 socket server
int initial_socket(int port, const char* ip)

// get rscq(remote syscall request)
int syscall_request_decode(struct rsc_header * header, char * buffer);

// execute rscq
int syscall_request_execute(struct rsc_header * header);

// 指针参数处理
int in_pointer_decode_server(struct rsc_header * header, char * buffer);
int out_pointer_decode_server(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);
int io_pointer_decode_server(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer);

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