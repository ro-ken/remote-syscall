#include "include/rsc_include.h"
#include "include/rsc_include_server.h"

int syscall_request_decode(int sockfd_s, struct rsc_regs * regs, struct pointer_manager * p_manager, unsigned int * syscall){
    int read_size = 0;
    int surplus_size = 0;
    int result = 0;
    char * buffer = NULL;
    struct rsc_header header;
    memset(&header, 0, sizeof(header));

    // 分两次读取远程系统调用请求, 先读取请求头获取整个请求的长度, 再读取剩下的请求体
    if ((read_size = read(sockfd_s, &header, sizeof(header))) < 0){
        printf("[server][error]: %d, %s in read header!\n", errno, strerror(errno));
        return -1;
    }
    surplus_size = header.size - sizeof(header);
    buffer = (char *)malloc(surplus_size);
    if ((read_size = read(sockfd_s, buffer, surplus_size)) < 0){
        printf("[server][error]: %d, %s in read surplus syscall request!\n", errno, strerror(errno));
        return -1;
    }

    // 提取 struct regs
    memcpy(regs, buffer, RSC_REGS_SIZE);

    // 获取远程系统调用分类
    *syscall = header.syscall;
    p_manager->p_flag = header.p_flag;

    switch (header.p_flag) {
        case 0: {
            result = no_pointer_server_decode(regs, p_manager);
        }
        case 1: {
            result = in_pointer_server_decode(regs, p_manager);
        }
        case 2: {
            result = out_pointer_server_decode(regs, p_manager);
        }
        case 3: {
            result = io_pointer_server_decode(regs, p_manager);
        }
    }

    if ( buffer != NULL){
        free(buffer);
        buffer = NULL;
    }

    return 1;
}

int no_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager){
    p_manager->p_addr_out = p_manager->p_addr_in = NULL;
    return 1;
}

int input_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer, unsigned int surplus_size){
    struct rsc_pointer pointer;
    memset(&pointer, 0, RSC_POINTER_SIZE);

    p_manager->p_addr_out = p_manager->p_addr_in = NULL;
    memcpy(&pointer, buffer, RSC_POINTER_SIZE);
    p_manager->p_location_in = pointer.p_location;
    p_manager->p_count_in = pointer.p_count;
    p_manager->p_addr_in = (char *)malloc(surplus_size - RSC_POINTER_SIZE);
    memset(p_manager->p_addr_in, 0, surplus_size - RSC_POINTER_SIZE);
    memcpy((char *)p_manager->p_addr_in, buffer + RSC_POINTER_SIZE, surplus_size - RSC_POINTER_SIZE);

    // 输入指针重定向
    switch (pointer.p_location) {
        case 1: regs->rdi = (unsigned long long int)p_manager->p_addr_in; break;
        case 2: regs->rsi = (unsigned long long int)p_manager->p_addr_in; break;
        case 3: regs->rdx = (unsigned long long int)p_manager->p_addr_in; break;
        case 4: regs->r10 = (unsigned long long int)p_manager->p_addr_in; break;
        case 5: regs->r8 = (unsigned long long int)p_manager->p_addr_in; break;
        case 6: regs->r9 = (unsigned long long int)p_manager->p_addr_in; break;
    }

    return 1;
}

int out_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer){
    struct rsc_pointer pointer;
    memset(&pointer, 0, RSC_POINTER_SIZE);

    p_manager->p_addr_out = p_manager->p_addr_in = NULL;
    memcpy(&pointer, buffer, RSC_POINTER_SIZE);
    p_manager->p_location_out = pointer.p_location;
    p_manager->p_count_out = pointer.p_count;
    p_manager->p_addr_out = (char *)malloc(pointer.p_count);
    memset(p_manager->p_addr_out, 0, pointer.p_count);

    // 输入指针重定向
    switch (pointer.p_location) {
        case 1: regs->rdi = (unsigned long long int)p_manager->p_addr_out; break;
        case 2: regs->rsi = (unsigned long long int)p_manager->p_addr_out; break;
        case 3: regs->rdx = (unsigned long long int)p_manager->p_addr_out; break;
        case 4: regs->r10 = (unsigned long long int)p_manager->p_addr_out; break;
        case 5: regs->r8 = (unsigned long long int)p_manager->p_addr_out; break;
        case 6: regs->r9 = (unsigned long long int)p_manager->p_addr_out; break;
    }

    return 1;
}

int io_pointer_server_decode(struct rsc_regs * regs, struct pointer_manager * p_manager, char * buffer){
    struct rsc_io_pointer io_pointer;
    memset(&pointer, 0, RSC_IO_POINTER_SIZE);

    p_manager->p_addr_out = p_manager->p_addr_in = NULL;
    memcpy(&io_pointer, buffer, RSC_IO_POINTER_SIZE);
    p_manager->p_location_in = io_pointer.p_location_in;
    p_manager->p_count_in = io_pointer.p_count_in;
    p_manager->p_location_out = io_pointer.p_location_out;
    p_manager->p_count_out = io_pointer.p_count_out;
    p_manager->p_addr_out = (char *)malloc(io_pointer.p_count_out);
    p_manager->p_addr_in = (char *)malloc(io_pointer.p_count_in);
    memset(p_manager->p_addr_out, 0, io_pointer.p_count_out);
    memset(p_manager->p_addr_in, 0, io_pointer.p_count_in);
    memcpy(p_manager->p_addr_in, buffer + RSC_IO_POINTER_SIZE, io_pointer.p_count_in);

    // 输入指针重定向
    switch (io_pointer.p_location_in) {
        case 1: regs->rdi = (unsigned long long int)p_manager->p_addr_in; break;
        case 2: regs->rsi = (unsigned long long int)p_manager->p_addr_in; break;
        case 3: regs->rdx = (unsigned long long int)p_manager->p_addr_in; break;
        case 4: regs->r10 = (unsigned long long int)p_manager->p_addr_in; break;
        case 5: regs->r8 = (unsigned long long int)p_manager->p_addr_in; break;
        case 6: regs->r9 = (unsigned long long int)p_manager->p_addr_in; break;
    }

    // 输出指针重定向
    switch (io_pointer.p_location_out) {
        case 1: regs->rdi = (unsigned long long int)p_manager->p_addr_out; break;
        case 2: regs->rsi = (unsigned long long int)p_manager->p_addr_out; break;
        case 3: regs->rdx = (unsigned long long int)p_manager->p_addr_out; break;
        case 4: regs->r10 = (unsigned long long int)p_manager->p_addr_out; break;
        case 5: regs->r8 = (unsigned long long int)p_manager->p_addr_out; break;
        case 6: regs->r9 = (unsigned long long int)p_manager->p_addr_out; break;
    }

    return 1;
}

// 远程系统调用请求执行
int syscall_execute(struct rsc_regs * regs, struct rsc_return_header * return_header, char * error_buffer, unsigned int * syscall){
    regs->rax = syscall(*syscall, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9);
    if (regs->rax < 0) {
        return_header->error = errno;
        return_header->error_size = strlen(strerror(errno));
        error_buffer = strerror(errno);
    }
    return 1;
}

/* 远程系统调用请求执行结果编组 */
int syscall_return_encode(int sockfd_s, struct rsc_regs * regs, struct pointer_manager * p_manager, char * syscall_return, unsigned int * syscall){
    
}
