/* remote syscall */
#include "../include/rsc.h"
#include "../include/rsc_client.h"

// syscall that can be executed remotely: 0, 1, 2, 3, 257


// remote syscall request encode, return a pointer to the syscall_reuqest buffer
int RequestEncode( struct user_regs_struct * regs, struct rsc_header * header, char * write_buffer, pid_t pid){
    // char *syscall_request = NULL;
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
    PointerEncode(header, write_buffer,pid);

    return 1;
}

// handle pointer parameters, return extra buffer pointer
int PointerEncode(struct rsc_header * header, char * write_buffer, pid_t pid){
    char * extra_buffer = NULL;         // 附加数据缓冲区
    // char * syscall_request = NULL;
    header->size = RSC_HEADER_SIZE;

    /* input pointer */
    switch(header->syscall) {
        case 1: {
            header->p_flag = IN_POINTER;
            extra_buffer = InputPointerEncode(2, header->rdx, header, pid);
            break;
        }
        case 2: {
            header->p_flag = IN_POINTER;
            extra_buffer = InputPointerEncode(1, 8, header, pid);
            break;
        }
        case 257: {
            header->p_flag = IN_POINTER;
            extra_buffer = InputPointerEncode(2, 8, header, pid);
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
            extra_buffer = InputPointerEncode(1, 8, header, pid);
            break;
        }
    }

    /* sequence input pointer */
    switch(header->syscall) {
        case 7: {}
    }

    memcpy(write_buffer, header, RSC_HEADER_SIZE);
    if (extra_buffer != NULL){
        memcpy(write_buffer + RSC_HEADER_SIZE, extra_buffer, header->size - RSC_HEADER_SIZE);
        free(extra_buffer);
        extra_buffer = NULL;
    }
    return 1;
}

char * InputPointerEncode(unsigned int p_location, unsigned int p_count, struct rsc_header * header, pid_t pid){
    unsigned long long int addr = 0;
    switch(p_location) {
        case 1: addr = header->rdi; break;
        case 2: addr = header->rsi; break;
        case 3: addr = header->rdx; break;
        case 4: addr = header->r10; break;
        case 5: addr = header->r8; break;
        case 6: addr = header->r9; break;
    }

    char * extra_buffer = NULL;

    header->size = p_count + RSC_HEADER_SIZE;
    header->p_location_in = p_location;
    header->p_count_in = p_count;
    
    extra_buffer = (char *)malloc(sizeof(char) * p_count);
    GetData(pid, addr, extra_buffer, p_count);

    return extra_buffer;
}

char * OutputPointerEncode(unsigned int p_location, unsigned int p_count, struct rsc_header * header){
    header->size = RSC_HEADER_SIZE;
    header->p_location_out = p_location;
    header->p_count_out = p_count;

    return NULL;
}

// remote syscall execute result decode
int ResultDecode(struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer,pid_t pid){
    switch (header->p_flag) {
        case 2: {
            if (OutputPointerDecode(regs, header, extra_buffer, pid) < 0) {
                printf("[client][ResultDecode]: in output pointer decode!\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
        case 3: {
            if (OutputPointerDecode(regs, header, extra_buffer, pid) < 0) {
                printf("[client][ResultDecode]: in output pointer decode!\n");
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    return 1;
}

int OutputPointerDecode( struct user_regs_struct * regs, struct rsc_header * header, char * extra_buffer, pid_t pid){
    printf("flags-1\n");
    switch (header->p_location_out) {

        case 1: PutData(pid, regs->rdi, extra_buffer, header->p_count_out); break;
        case 2: PutData(pid, regs->rsi, extra_buffer, header->p_count_out); break;
        case 3: PutData(pid, regs->rdx, extra_buffer, header->p_count_out); break;
        case 4: PutData(pid, regs->r10, extra_buffer, header->p_count_out); break;
        case 5: PutData(pid, regs->r8, extra_buffer, header->p_count_out); break;
        case 6: PutData(pid, regs->r9, extra_buffer, header->p_count_out); break;
    }

    return 1;
}
 
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

int IsSet(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return -1;
    }
    int base = syscall / LLSIZE;    
    int surplus = syscall % LLSIZE; 

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    base_n = (base_n >> surplus) | ISSET_MASK;
    return base_n==0xffffffffffffffff?1:-1;
}

void ResetBitmap(unsigned long long int * syscall_bitmap, unsigned int syscall){
    if (syscall < 0 && syscall > 547){
        return;
    }
    int base = syscall / LLSIZE;    
    int surplus = syscall % LLSIZE; 

    unsigned long long int * base_p = syscall_bitmap + base;
    unsigned long long int base_n = *base_p;
    *base_p = base_n | ((( base_n >> surplus) & RESET_MASK) << surplus);
}

void GetData(pid_t child, unsigned long addr, char *str, int len){
    char *laddr;
    int i, j;
    union u {
            long val;
            char chars[8];
    }data;
    i = 0;
    j = len / 8;
    laddr = str;
    while(i < j) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 4,
                          NULL);
        memcpy(laddr, data.chars, 8);
        ++i;
        laddr += 8;
    }
    j = len % 8;
    if(j != 0) {
        data.val = ptrace(PTRACE_PEEKDATA,
                          child, addr + i * 4,
                          NULL);
        memcpy(laddr, data.chars, j);
    }
    str[len] = '\0';
}

void PutData(pid_t child, unsigned long addr, char *str, int len){   
    char *laddr;
    int i, j;
    union u {
            long val;
            char chars[8];
    }data;
    i = 0;
    j = len / 8;
    laddr = str;
    while(i < j) {
        memcpy(data.chars, laddr, 8);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
        ++i;
        laddr += 8;
    }
    j = len % 8;
    if(j != 0) {
        memcpy(data.chars, laddr, j);
        ptrace(PTRACE_POKEDATA, child,
               addr + i * 4, data.val);
    }
}

void DebugPrintf(struct rsc_header * header){
    fprintf(stderr, "syscall:%ld, p_flag:%d, size:%d, error:%d\n", header->syscall,header->p_flag, header->size, header->error);
    fprintf(stderr, "rax:%lld, rdi:%lld, rsi:%lld, rdx:%lld, r10:%lld, r8:%lld, r9:%lld\n", header->rax, header->rdi,header->rsi,header->rdx,header->r10,header->r8,header->r9);
    fprintf(stderr, "p_location_in:%d, p_location_out:%d, p_count_in:%d, p_count_out:%d\n", header->p_location_in, header->p_location_out, header->p_count_in, header->p_count_out);
    fprintf(stderr, "p_addr_in:%s, p_addr_out:%s\n", header->p_addr_in, header->p_addr_out);
}