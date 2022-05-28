#include "rsc.h"
#include "rsc_syscall_classify.h"

// 远程系统调用请求编组
char * syscall_request_encode(struct rsc_header *header, struct user_regs_struct *user_regs){
    unsigned int buffer_size = 0;   // 附加数据缓冲区大小
    char * buffer = NULL;           // 附加数据缓冲区
    char * syscall_request = NULL;  // 远程系统调用请求

    // 初始化 struct rsc_header 
    header->syscall = user_regs->orig_rax;
    header->rax = user_regs->rax;
    header->rdi = user_regs->rdi;
    header->rsi = user_regs->rsi;
    header->rdx = user_regs->rdx;
    header->r10 = user_regs->r10;
    header->r8 = user_regs->r8;
    header->r9 = user_regs->r9;
    header->p_addr_in = NULL;
    header->p_addr_out = NULL;

    // 处理不同的指针参数, 返回附加数据缓冲区指针
    buffer = pointer_encode_client(header);

    // 填充 RSCQ(remote syscall request)
    syscall_request = (char *)malloc(sizeof(char) * header->size);
    memcpy(syscall_request, header, RSC_HEADER_SIZE);
    if (buffer != NULL){
        memcpy(syscall_request + RSC_HEADER_SIZE, buffer, header->size - RSC_HEADER_SIZE);
        free(buffer);
        buffer = NULL;
    }

    // 记住之后要 free(syscall_request) 
    return syscall_request;
}

int syscall_return_decode(struct user_regs_struct * u_regs, struct rsc_header * header, char * syscall_result){
    switch (header->p_flag) {
        case 0: {
            break;
        }
        case 1: {
            break;
        }
        case 2: {
            if (out_pointer_decode_client(u_regs, header, syscall_result) < 0){
                printf("[client][socket]: %d, %s, in build connect. server ip: %s, server_port is: %d\n", errno, strerror(errno), ip_addr, port);
                return -1;
            }
            break;
        }
        case 3: {
            break;
        }
        case 4: {
            break;
        }
    }
}

// 根据系统调用号处理指针参数
char * pointer_encode_client(struct rsc_header * header){
    char * buffer = NULL;         // 附加数据缓冲区

    /* 处理带输入指针参数的系统调用 */
    switch(header->syscall) {
        case 1: {
            header->p_flag = IN_POINTER;
            buffer = in_pointer_encode_client(2, header->rdx, header->rsi, header);
            break;
        }
        case 2: {
            header->p_flag = IN_POINTER;
            buffer = in_pointer_encode_client(1, strlen((char *)header->rdi), header->rdi, header);
            break;
        }
        case 257: {
            header->p_flag = IN_POINTER;
            buffer = in_pointer_encode_client(2, strlen((char *)header->rdi), header->rdi, header);
            break;
        }
    }

    /* 处理带输出指针参数的系统调用 */
    switch(header->syscall) {
        case 0: {
            header->p_flag = OUTPUT_POINTER;   
            buffer = out_pointer_encode_client(2, header->rdx, header);
            break;
        }
    }

    /* 处理带输入输出指针参数的系统调用 */
    switch(header->syscall) {
        case 89: {
            header->p_flag =IO_POINTER;
            buffer = out_pointer_encode_client(2, header->rdx, header);
            buffer = in_pointer_encode_client(1, strlen((char *)header->rdi), header->rdi, header);
            break;
        }
    }

    /* 处理带连续输入指针参数的系统调用 */
    switch(header->syscall) {
        case 7: {}
    }

    return buffer;
}

// 带输入指针参数的系统调用,
// 从输入指针指向的内存中取出指定大小的数据附加到 RSCQ 同步服务端数据
char * in_pointer_encode_client(unsigned int p_location, unsigned int p_count, unsigned long long int addr, struct rsc_header * header){
    char * buffer = NULL;

    header->size = p_count + RSC_HEADER_SIZE;
    header->p_location_in = p_location;
    header->p_count_in = p_count;

    buffer = (char *)malloc(sizeof(char) * p_count);
    memcpy(buffer, (char *)addr, p_count);

    return buffer;
}

// 带输出指针参数的系统调用
// 本地端不作为, 服务端需要对 RSCQ 执行结果进行处理
char * out_pointer_encode_client(unsigned int p_location, unsigned int p_count, struct rsc_header * header){
    header->size = RSC_HEADER_SIZE;
    pointer->p_location_out = p_location;
    pointer->p_count_out = p_count;

    return NULL;
}

int out_pointer_decode_client(struct user_regs_struct * u_regs, struct rsc_header * header, char * syscall_result){
    switch (header->p_location_out) {
        case 1: memcpy((char *)u_regs->rdi, syscall_result, header->p_count_out); break;
        case 2: memcpy((char *)u_regs->rsi, syscall_result, header->p_count_out); break;
        case 3: memcpy((char *)u_regs->rdx, syscall_result, header->p_count_out); break;
        case 4: memcpy((char *)u_regs->r10, syscall_result, header->p_count_out); break;
        case 5: memcpy((char *)u_regs->r8, syscall_result, header->p_count_out); break;
        case 6: memcpy((char *)u_regs->r9, syscall_result, header->p_count_out); break;
    }
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



