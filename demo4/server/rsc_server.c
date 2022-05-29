#include "../include/rsc_include.h"
#include "../include/rsc_include_server.h"

int syscall_request_decode(struct rsc_header * header, char * buffer){
    switch (header->p_flag) {
        case 0: {
            break;
        }
        case 1: {
            if (in_pointer_decode_server(header, buffer) < 0){
                printf("[server][decode][in]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 2: {
            if (out_pointer_decode_server(header) < 0){
                printf("[server][decode][out]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 3: {
            if (in_pointer_decode_server(header, buffer) < 0){
                printf("[server][decode][io-in]: error, will exit!\n");
                return -1;
            }
            if (out_pointer_decode_server(header) < 0){
                printf("[server][decode][io-out]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 4: {
            break;
        }
    }
}

// 输入指针重定向
int in_pointer_decode_server(struct rsc_header * header, char * buffer){
    header->p_addr_in = buffer;

    // 输入指针重定向
    switch (header->p_location_in) {
        case 1: header->rdi = (unsigned long long int)buffer; break;
        case 2: header->rsi = (unsigned long long int)buffer; break;
        case 3: header->rdx = (unsigned long long int)buffer; break;
        case 4: header->r10 = (unsigned long long int)buffer; break;
        case 5: header->r8 = (unsigned long long int)buffer; break;
        case 6: header->r9 = (unsigned long long int)buffer; break;
    }

    return 1;
}

// 输出指针重定向
int out_pointer_decode_server(struct rsc_header * header){
    char * out_buffer = NULL;
    out_buffer = (char *)malloc(sizeof(char) * header->p_count_out);
    memset(out_buffer, 0, header->p_count_out);
    header->p_addr_out = out_buffer;

    // 输入指针重定向
    switch (header->p_location_out) {
        case 1: header->rdi = (unsigned long long int)out_buffer; break;
        case 2: header->rsi = (unsigned long long int)out_buffer; break;
        case 3: header->rdx = (unsigned long long int)out_buffer; break;
        case 4: header->r10 = (unsigned long long int)out_buffer; break;
        case 5: header->r8 = (unsigned long long int)out_buffer; break;
        case 6: header->r9 = (unsigned long long int)out_buffer; break;
    }

    return 1;
}

// 远程系统调用请求执行
int syscall_request_execute(struct rsc_header * header){
    header->rax = syscall(header->syscall, header->rdi, header->rsi, header->rdx, header->r10, header->r8, header->r9);
    if (header->rax < 0) {
        header->error = errno;
    }
    return 1;
}

/* 远程系统调用请求执行结果编组 */
char * syscall_return_encode(struct rsc_header * header){
    char * syscall_result = NULL;
    switch(header->p_flag) {
        case 0: {
            header->size = RSC_HEADER_SIZE;
            syscall_result = (char *)malloc(sizeof(char *) * RSC_HEADER_SIZE);
            memcpy(syscall_result, header, RSC_HEADER_SIZE);
            break;
        }
        case 1: {
            header->size = RSC_HEADER_SIZE;
            syscall_result = (char *)malloc(sizeof(char *) * RSC_HEADER_SIZE);
            memcpy(syscall_result, header, RSC_HEADER_SIZE);
            break;
        }
        case 2: {
            syscall_result = out_pointer_encode_server(header);
            break;
        }
        case 3: {
            syscall_result = out_pointer_encode_server(header);
            break;
        }
        case 4: {
            break;
        }
    }
    return syscall_result;
}

char * out_pointer_encode_server(struct rsc_header * header){
    char * syscall_result = NULL;
    header->size = RSC_HEADER_SIZE + header->p_count_out;
    syscall_result = (char *)malloc(sizeof(char) * header->size);

    memcpy(syscall_result, header, RSC_HEADER_SIZE);
    memcpy(syscall_result + RSC_HEADER_SIZE, header->p_addr_out, header->p_count_out);
    return syscall_result; 
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
