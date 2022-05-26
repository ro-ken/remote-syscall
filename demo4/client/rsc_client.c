#include "rsc.h"
#include "rsc_syscall_classify.h"


// 远程系统调用请求编组
char * syscall_request_encode(struct rsc_header *header, struct rsc_regs *regs, struct user_regs_struct *user_regs){
    unsigned int buffer_size = 0;   // 附加数据缓冲区大小
    char * buffer = NULL;           // 附加数据缓冲区
    char * syscall_request = NULL;  // 远程系统调用请求

    // 初始化 struct rsc_header 和 struct rsc_regs 
    header->syscall = user_regs->orig_rax;
    regs->rax = user_regs->rax;
    regs->rdi = user_regs->rdi;
    regs->rsi = user_regs->rsi;
    regs->rdx = user_regs->rdx;
    regs->r10 = user_regs->r10;
    regs->r8 = user_regs->r8;
    regs->r9 = user_regs->r9;

    // 处理指针参数, 根据系统调用分类填充附加数据缓冲区，返回附加数据缓冲区指针
    buffer = pointer_encode_client(header, regs, &buffer_size);

    // 填充远程系统调用请求
    *size = header->size = buffer_size + RSC_HEADER_SIZE + RSC_REGS_SIZE;
    syscall_request = (char *)malloc(header->size);
    memset(syscall_request, 0, header->size);
    memcpy(syscall_request, &header, RSC_HEADER_SIZE);
    memcpy(syscall_request + RSC_HEADER_SIZE, &regs, RSC_REGS_SIZE);
    if (buffer != NULL){
        memcpy(syscall_request + RSC_HEADER_SIZE + RSC_REGS_SIZE, buffer, buffer_size);
        free(buffer);
        buffer = NULL;
    }

    // 记住之后要 free(syscall_request) 
    return syscall_request;
}

/* 系统调用指针参数处理 */
char * pointer_client(struct rsc_header * header, struct rsc_regs * regs, unsigned int * buffer_size){
    char * buffer = NULL;                                   // 附加数据缓冲区

    /* 处理带输入指针参数的系统调用 */
    switch(header->syscall) {
        case 1: {
            header->p_flag = IN_POINTER;
            buffer = in_pointer_encode_client(2, regs.rdx, regs.rsi, buffer_size);
        }
        case 2: {
            header->p_flag = INPUT_POINTER;
            buffer = input_pointer_handle(1, strlen((char *)regs.rdi), regs.rdi, buffer_size);
        } 
        // case 18: {
        //     header->p_flag = INPUT_POINTER;
        //     buffer = input_pointer_handle(2, regs.rdx, regs.rsi, buffer_size);
        // }
        // case 87: {
        //     header->p_flag = INPUT_POINTER;
        //     buffer = input_pointer_handle(1, strlen((char *)regs.rdi), regs.rdi, buffer_size);
        // }
        // case 237: {
        //     header->p_flag = INPUT_POINTER;
        //     buffer = input_pointer_handle(2, strlen((char *)regs.rsi), regs.rsi, buffer_size);
        // }
    }

    /* 处理带输出指针参数的系统调用 */
    switch(syscall) {
        case 0: {
            header->p_flag = OUTPUT_POINTER;     // 标识系统调用分类
            buffer = output_pointer_handle(2, regs.rdx, buffer_size);
        }
        case 5: {
            header->p_flag = OUTPUT_POINTER;     // 标识系统调用分类
            buffer = output_pointer_handle(2, sizeof(struct stat), buffer_size);
        }
        case 17: {
            header->p_flag = OUTPUT_POINTER;     // 标识系统调用分类
            buffer = output_pointer_handle(2, regs.rdx, buffer_size);
        }
        case 138: {
            header->p_flag = OUTPUT_POINTER;     // 标识系统调用分类
            buffer = output_pointer_handle(2, sizeof(struct statfs), buffer_size);
        }
    }

    /* 处理带输入输出指针参数的系统调用 */
    switch(syscall) {
        case 89: {
            header->p_flag =IO_POINTER 3;     // 标识系统调用分类
            struct io_rsc_pointer io_pointer;
            memset(&io_pointer, 0, sizeof(io_pointer));

            io_pointer.p_location_in = 1;
            io_pointer.p_location_out = 2;
            io_pointer.p_count_in = strlen((char *)regs.rdi);
            io_pointer.p_count_out = regs.rdx;
            io_pointer.addr_in = regs.rsi;
            buffer = io_pointer_handle(&io_pointer, buffer_size);
        }
    }

    /* 处理带连续输入指针参数的系统调用 */
    switch(syscall) {
        case 7: {}
    }

    return buffer;
}

/* 带输出指针参数的系统调用处理 */
char * output_pointer_handle(unsigned int p_location, unsigned int p_count, unsigned int * buffer_size){
    // 使用 struct rsc_pointer 描述指针参数信息
    struct rsc_pointer * pointer = NULL;
    pointer = (struct rsc_pointer *)malloc(RSC_POINTER_SIZE);

    // 填充 struct rsc_pointer, 将其作为附加缓冲区扩展到远程系统调用请求中
    *buffer_size = RSC_POINTER_SIZE;
    pointer->p_location = p_location;
    pointer->p_count = p_count;

    return pointer;
}

/* 带输入指针参数的系统调用处理 */
char * in_pointer_encode_client(unsigned int p_location, unsigned int p_count, unsigned long long int addr, unsigned int * buffer_size){
    // 使用 struct rsc_pointer 描述指针参数信息
    struct rsc_pointer pointer;
    memset(&pointer, 0, sizeof(pointer));

    // malloc一片内存区作为附加数据缓冲区, 其中包含 struct rsc_pointer 和输入指针参数指向的内存区数据
    char * buffer = NULL;
    buffer = (char *)malloc(RSC_POINTER_SIZE + p_count);

    // 填充附加数据缓冲区
    *buffer_size = RSC_POINTER_SIZE + p_count;
    pointer.p_location = p_location;
    pointer.p_count = p_count;
    memcpy(buffer, &pointer, RSC_POINTER_SIZE);
    memcpy(buffer + RSC_POINTER_SIZE, (char *)addr, p_count);

    return buffer;
}

/* 带输入输出指针参数的系统调用处理 */
char * io_pointer_handle(struct io_rsc_pointer * io_pointer,  unsigned int * buffer_size){
    // 使用 struct rsc_pointer 描述指针参数信息
    struct rsc_pointer pointer_in;
    memset(&pointer_in, 0, sizeof(pointer_in));
    struct rsc_pointer pointer_out;
    memset(&pointer_out, 0, sizeof(pointer_out));

    pointer_in.p_location = io_pointer->p_location_in;
    pointer_in.p_count = io_pointer->p_count_in;
    pointer_out.p_location = io_pointer->p_location_out;
    pointer_out.p_count = io_pointer->p_count_out;

    // malloc一片内存区作为附加数据缓冲区, 其中包含 struct rsc_pointer 和输入指针参数指向的内存区数据
    char * buffer = NULL;
    buffer = (char *)malloc(RSC_POINTER_SIZE * 2 + p_count);

    // 填充附加数据缓冲区
    *buffer_size = RSC_POINTER_SIZE * 2 + io_pointer->p_count_in;
    memcpy(buffer, &pointer_in, RSC_POINTER_SIZE);
    memcpy(buffer + RSC_POINTER_SIZE, &pointer_out, RSC_POINTER_SIZE);
    memcpy(buffer + RSC_POINTER_SIZE * 2, (char *)io_pointer->addr_in, io_pointer->p_count_in);

    return buffer;
}

// 在客户端创建一个socket连接，返回socket文件描述符 
int client_connect_socket(char* ip_addr, int port){
    struct sockaddr_in server_addr;
    int sockfd = -1;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("[client][socket]: %d, %s, in create client socket!\n", errno, strerror(errno));
        return -1;
    }

    if(connect(sockfd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr)) < 0){
        printf("[client][socket]: %d, %s, in build connect. server ip: %s, server_port is: %d\n", errno, strerror(errno), ip_addr, port);
        return -1;
    }

    return sockfd;
}



