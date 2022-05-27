#include "include/rsc_include.h"
#include "include/rsc_include_server.h"

int syscall_request_decode(struct rsc_header * header, char * buffer){
    switch (header.p_flag) {
        case 0: {
            break;
        }
        case 1: {
            if (in_pointer_decode_server(struct rsc_header * header, char * buffer) < 0){
                printf("[server][decode][in]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 2: {
            if (out_pointer_decode_server(struct rsc_header * header) < 0){
                printf("[server][decode][out]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 3: {
            if (io_pointer_decode_server(struct rsc_header * header, char * buffer) < 0){
                printf("[server][decode][out]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 4: {
            break;
        }
    }
}

int in_pointer_decode_server(struct rsc_header * header, char * buffer){
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

int out_pointer_decode_server(struct rsc_header * header){
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

int io_pointer_decode_server(struct rsc_header * header, char * buffer){
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

// 建立socket server
int initial_socket(int port, const char* ip)
{
    int sockfd = -1;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("[server]][socket]: errno, %d, strerror: %s, first syscall\n", errno, strerror(errno));
        return -1;
    }

    int opt=1;
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    socklen_t len = sizeof(server);

    if(bind(sockfd,(struct sockaddr*)&server , len) < 0)
    {
        printf("[server]][bind]: errno, %d, strerror: %s, first syscall\n", errno, strerror(errno));
        return -1;
    }

    if(listen(sockfd, 5) < 0)    //允许连接的最大数量为5
    {
        printf("[server]][listen]: errno, %d, strerror: %s, first syscall\n", errno, strerror(errno));
        return -1;
    }

    return sockfd;
}
