/* remote syscall */
#include "../include/rsc.h"
#include "../include/rsc_client.h"

// syscall that can be executed remotely: 0, 1, 2, 3, 257
unsigned long long syscall_bitmap[9] = {15, 0, 0, 0, 3, 0, 0, 0, 0};

// remote syscall request encode, return a pointer to the syscall_reuqest buffer
char * RequestEncode( struct user_regs_struct * regs, struct rsc_header * header){
    char * extra_buffer = NULL;           // extra buffer
    char * syscall_request = NULL;  // pointer to remote syscall request buffer

    // initial struct header
    header->syscall = regs->orig_rax;
    header->rax = regs->rax;
    header->rdi = regs->rdi;
    header->rsi = regs->rsi;
    header->rdx = regs->rdx;
    header->r10 = regs->r10;
    header->r8 = regs->r8;
    header->r9 = regs->r9;
    header->p_addr_in = NULL;
    header->p_addr_out = NULL;

    // encode pointer paraments, memory data stored in extra buffer
    extra_buffer = PointerEncode(header);

    // fill remote syscall request buffer(cover struct rsc_header and extra buffer)
    syscall_request = (char *)malloc(sizeof(char) * header->size);
    memcpy(syscall_request, header, RSC_HEADER_SIZE);
    if (extra_buffer != NULL){    // extra buffer exsit, fill it to remote syscall request buffer
        memcpy(syscall_request + RSC_HEADER_SIZE, extra_buffer, header->size - RSC_HEADER_SIZE);
        free(extra_buffer);
        extra_buffer = NULL;
    }

    return syscall_request;
}

// handle pointer parameters, return extra buffer pointer
char * PointerEncode(struct rsc_header * header){
    char * extra_buffer = NULL;         // 附加数据缓冲区

    /* input pointer */
    switch(header->syscall) {
        case 1: {
            header->p_flag = IN_POINTER;
            extra_buffer = InputPointerEncode(2, header->rdx, header->rsi, header);
            break;
        }
        case 2: {
            header->p_flag = IN_POINTER;
            extra_buffer = InputPointerEncode(1, strlen((char *)header->rdi), header->rdi, header);
            break;
        }
        case 257: {
            header->p_flag = IN_POINTER;
            extra_buffer = InputPointerEncode(2, strlen((char *)header->rdi), header->rdi, header);
            break;
        }
    }

    /* output pointer */
    switch(header->syscall) {
        case 0: {
            header->p_flag = OUT_POINTER;   
            extra_buffer = OutputPointerEncode(2, header->rdx, header);
            break;
        }
    }

    /* IO pointer */
    switch(header->syscall) {
        case 89: {
            header->p_flag =IO_POINTER;
            extra_buffer = OutputPointerEncode(2, header->rdx, header);
            extra_buffer = InputPointerEncode(1, strlen((char *)header->rdi), header->rdi, header);
            break;
        }
    }

    /* sequence input pointer */
    switch(header->syscall) {
        case 7: {}
    }

    return extra_buffer;
}

char * InputPointerEncode(unsigned int p_location, unsigned int p_count, unsigned long long addr_in, struct rsc_header * header){
    char * extra_buffer = NULL;

    header->size = p_count + RSC_HEADER_SIZE;
    header->p_location_in = p_location;
    header->p_count_in = p_count;

    extra_buffer = (char *)malloc(sizeof(char) * p_count);
    memcpy(extra_buffer, (char *)addr_in, p_count);

    return extra_buffer;
}

char * OutputPointerEncode(unsigned int p_location, unsigned int p_count, struct rsc_header * header){
    header->size = RSC_HEADER_SIZE;
    header->p_location_out = p_location;
    header->p_count_out = p_count;

    return NULL;
}

// remote syscall execute result decode
int ResultDecode(struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer){
    switch (header->p_flag) {
        case 2: {
            if (OutputPointerDecode(regs, header, extra_buffer) < 0) {
                printf("[client][ResultDecode]: in output pointer decode!\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 3: {
            if (OutputPointerDecode(regs, header, extra_buffer) < 0) {
                printf("[client][ResultDecode]: in output pointer decode!\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    return 1;
}

int OutputPointerDecode( struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer){
    switch (header->p_location_out) {
        case 1: memcpy((char *)regs->rdi, extra_buffer, header->p_count_out); break;
        case 2: memcpy((char *)regs->rsi, extra_buffer, header->p_count_out); break;
        case 3: memcpy((char *)regs->rdx, extra_buffer, header->p_count_out); break;
        case 4: memcpy((char *)regs->r10, extra_buffer, header->p_count_out); break;
        case 5: memcpy((char *)regs->r8, extra_buffer, header->p_count_out); break;
        case 6: memcpy((char *)regs->r9, extra_buffer, header->p_count_out); break;
    }

    return 1;
}

// 在客户端创建一个socket连接，返回socket文件描述符 
int SocketConnect(char* ip_addr, int port){
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

// 置位系统调用号对应的位示图bit位
void SetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return;
    }
    int base = syscall / LLSIZE;    // 获取基址
    int surplus = syscall % LLSIZE; // 余值

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    *base_p = base_n | ((( base_n >> surplus) | SET_MASK) << surplus);
}

// 查询当前系统调用是否已实现
int IsSet(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return -1;
    }
    int base = syscall / LLSIZE;    // 获取基址
    int surplus = syscall % LLSIZE; // 余值

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    base_n = (base_n >> surplus) | ISSET_MASK;
    return base_n==0xffffffffffffffff?1:-1;
}

// 置位系统调用号对应的位示图bit位
void ResetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return;
    }
    int base = syscall / LLSIZE;    // 获取基址
    int surplus = syscall % LLSIZE; // 余值

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    *base_p = base_n | ((( base_n >> surplus) & RESET_MASK) << surplus);
}