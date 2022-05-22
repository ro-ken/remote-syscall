#include "rsc.h"


// 远程系统调用请求编组
void syscall_request_encode(struct user_regs_struct * regs){

}

// 远程系统调用请求解组
void syscall_request_decode();

// 远程系统调用执行结果编组
void syscall_return_encode();

// 远程系统调用执行结果解组
void syscall_return_decode();


/* 系统调用指针表 */
void syscall_point_query(char * syscall_request){
    
}