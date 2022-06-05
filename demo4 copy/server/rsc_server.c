#include "../include/rsc.h"
#include "../include/rsc_server.h"

int RSCHandle(int sockfd_c){

    // initial auxiliary data struct
    char * extra_buffer = NULL;
    char * syscall_result = NULL;
    struct rsc_header header;
    memset(&header, 0, RSC_HEADER_SIZE);

    // loop handle remote syscall request
    for(;;){
        if(read(sockfd_c, &header, RSC_HEADER_SIZE) < 0)
            FATAL("[%s][%s]: %s", "server", "read", strerror(errno));
        if (header.size > RSC_HEADER_SIZE){
            extra_buffer = (char *)malloc(sizeof(char) * (header.size - RSC_HEADER_SIZE));
            if(read(sockfd_c, extra_buffer, header.size - RSC_HEADER_SIZE) < 0)
                FATAL("[%s][%s]: %s, in read extra_buffer!", "server", "read", strerror(errno));
        }

        // remtoe syscall request decode
        if (RequestDecode(&header, extra_buffer) < 0)
            FATAL("[%s][%s]: remote syscall request decode failure!", "server", "RequestDecode");

        // execute remote syscall request
        if (RequestExecute(&header) < 0)
            FATAL("[%s][%s]: remote syscall request execute failure!", "server", "RequestExecute");

        // remote syscall request execute result encode
        syscall_result = ResultEncode(&header);

        // return rscq result
        if (write(sockfd_c, syscall_result, header.size) < 0) 
            FATAL("[%s][%s]: %s", "server", "write", strerror(errno));

        // Handling the crime scene
        free(syscall_result);
        if (header.p_addr_in != NULL) free(header.p_addr_in);
        if (header.p_addr_out != NULL) free(header.p_addr_out);
        extra_buffer = NULL;
        syscall_result = NULL;
        memset(&header, 0, RSC_HEADER_SIZE);
    }
}


int RequestDecode(struct rsc_header * header, char * buffer){
    DebugPrintf(header);
    switch (header->p_flag) {
        case 0: {
            break;
        }
        case 1: {
            if (InputPointerDecode(header, buffer) < 0){
                printf("[server][InputPointerDecode]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 2: {
            if (OutputPointerDecode(header) < 0){
                printf("[server][OutputPointerDecode]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 3: {
            if (InputPointerDecode(header, buffer) < 0){
                printf("[server][InputPointerDecode]: error, will exit!\n");
                return -1;
            }
            if (OutputPointerDecode(header) < 0){
                printf("[server][OutputPointerDecode]: error, will exit!\n");
                return -1;
            }
            break;
        }
        case 4: {
            break;
        }
    }

    return 1;
}

int InputPointerDecode(struct rsc_header * header, char * buffer){
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

int OutputPointerDecode(struct rsc_header * header){
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


int RequestExecute(struct rsc_header * header){
    header->rax = syscall(header->syscall, header->rdi, header->rsi, header->rdx, header->r10, header->r8, header->r9);
    if (header->rax < 0) {
        header->error = errno;
    }
    return 1;
}

char * ResultEncode(struct rsc_header * header){
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
            syscall_result = OutputPointerEncode(header);
            break;
        }
        case 3: {
            syscall_result = OutputPointerEncode(header);
            break;
        }
        case 4: {
            break;
        }
    }
    return syscall_result;
}

char * OutputPointerEncode(struct rsc_header * header){
    char * syscall_result = NULL;
    header->size = RSC_HEADER_SIZE + header->p_count_out;
    syscall_result = (char *)malloc(sizeof(char) * header->size);

    memcpy(syscall_result, header, RSC_HEADER_SIZE);
    memcpy(syscall_result + RSC_HEADER_SIZE, header->p_addr_out, header->p_count_out);
    return syscall_result; 
}

int InitialSocket(int port, const char* ip)
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

void DebugPrintf(struct rsc_header * header){
    fprintf(stderr, "syscall:%lld, p_flag:%d, size:%d, error:%d\n", header->syscall,header->p_flag, header->size, header->error);
    fprintf(stderr, "rax:%lld, rdi:%lld, rsi:%lld, rdx:%lld, r10:%lld, r8:%lld, r9:%lld\n", header->rax, header->rdi,header->rsi,header->rdx,header->r10,header->r8,header->r9);
    fprintf(stderr, "p_location_in:%d, p_location_out:%d, p_count_in:%d, p_count_out:%d\n", header->p_location_in, header->p_location_out, header->p_count_in, header->p_count_out);
    fprintf(stderr, "p_addr_in:%s, p_addr_out:%s\n", header->p_addr_in, header->p_addr_out);
}