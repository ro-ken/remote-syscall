#include "rsc.h"
#include "rsc_syscall_classify.h"


// 远程系统调用请求编组
int syscall_request_encode(struct user_regs_struct * user_regs, void * syscall_request){
    unsigned int size = 0;  // 远程系统调用请求的长度，用于通知socket发送多少字节

    /* 定义局部变量，存放部分远程系统调用请求内容 */
    struct rsc_header header;
    struct rsc_regs regs;
    memset(&header, 0, RSC_HEADER_SIZE);
    memset(&regs, 0, RSC_REGS_SIZE);

    /* initial struct rsc_header and struct rsc_regs */
    header.syscall = user_regs.orig_rax;
    regs.rax = user_regs.rax;
    regs.rdi = user_regs.rdi;
    regs.rsi = user_regs.rsi;
    regs.rdx = user_regs.rdx;
    regs.r10 = user_regs.r10;
    regs.r8 = user_regs.r8;
    regs.r9 = user_regs.r9;

    /* 根据系统调用的分类分别处理 */
    size = syscall_pointer_handle(&header, &regs, syscall_request);


}

// 远程系统调用请求解组
void syscall_request_decode();

// 远程系统调用执行结果编组
void syscall_return_encode();

// 远程系统调用执行结果解组
void syscall_return_decode();



