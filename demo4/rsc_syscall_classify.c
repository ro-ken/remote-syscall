#include "rsc.h"
#include "rsc_syscall_classify.h"

/* 处理系统调用的指针参数 */
int syscall_pointer_handle(struct rsc_header * header, struct rsc_regs * regs, void * syscall_request){
    unsigned int syscall = (unsigned int)header->syscall;   // 系统调用号
    unsigned int size = 0;                                  // 远程系统调用请求大小
    unsigned int buffer_size = 0;                           // 附加数据缓冲区大小
    char * buffer = NULL;                                   // 附加数据缓冲区

    /* 处理带输入指针参数的系统调用 */
    switch(syscall) {
        case 1: {}
        case 2: {}
        case 18: {}
        case 87: {}
        case 237: {}
    }

    /* 处理带输出指针参数的系统调用 */
    switch(syscall) {
        case 0: {
            header->p_flag = 2;     // 标识系统调用分类
            
        }
        case 5: {}
        case 17: {}
        case 138: {}
    }

    /* 处理带输入输出指针参数的系统调用 */
    switch(syscall) {
        case 89: {}
    }

    /* 处理带连续输入指针参数的系统调用 */
    switch(syscall) {
        case 7: {}
    }

    /* 填充请求头和寄存器参数 */
    header->p_number = count;
    header->size = RSC_HEADER_SIZE + RSC_REGS_SIZE + size;
    memcpy(syscall_request, header, RSC_HEADER_SIZE);
    memcpy((char *)syscall_request + RSC_HEADER_SIZE, regs, RSC_REGS_SIZE);

    /* 在远程系统调用请求后面扩充指针头和附加数据缓冲区 */
    if( size != 0){
        syscall_request = realloc(syscall_request, RSC_HEADER_SIZE + RSC_REGS_SIZE + size);
        memcpy((char *)syscall_request + RSC_HEADER_SIZE + RSC_REGS_SIZE, pointer, size);
    }
}