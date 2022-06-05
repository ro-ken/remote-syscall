#include "../include/rsc.h"
#include "../include/rsc_server.h"

int RSCHandle(int sockfd_c){

    // initial auxiliary data struct
    char * extra_buffer = NULL;
    struct rsc_header header;
    memset(&header, 0, RSC_HEADER_SIZE);

    // loop handle remote syscall request
    for(;;){
        int ret = 0;
        char read_buffer[1000];
        if((ret = read(sockfd_c, read_buffer, 1000)) < 0)
            FATAL("[%s][%s]: %s", "server", "read", strerror(errno));
        memcpy(&header, read_buffer, RSC_HEADER_SIZE);
        DebugPrintf(&header);

        if (header.size > RSC_HEADER_SIZE){
            extra_buffer = (char *)malloc(sizeof(char) * (header.size - RSC_HEADER_SIZE));
            memcpy(extra_buffer, read_buffer + RSC_HEADER_SIZE, header.size - RSC_HEADER_SIZE);
        }

        // remtoe syscall request decode
        if (RequestDecode(&header, extra_buffer) < 0)
            FATAL("[%s][%s]: remote syscall request decode failure!", "server", "RequestDecode");
        
        // Debug check struct rsc_header
        DebugPrintf(&header);

        // execute remote syscall request
        if (RequestExecute(&header) < 0)
            FATAL("[%s][%s]: remote syscall request execute failure!", "server", "RequestExecute");

        // remote syscall request execute result encode
        memset(read_buffer, 0 , 1000);
        ResultEncode(&header, read_buffer);

        // // Debug check struct rsc_header
        // DebugPrintf(&header);

        // return rscq result
        if (write(sockfd_c, read_buffer, 1000) < 0) 
            FATAL("[%s][%s]: %s", "server", "write", strerror(errno));

        // Handling the crime scene
        if (header.p_addr_in != NULL) free(header.p_addr_in);
        if (header.p_addr_out != NULL) free(header.p_addr_out);
        extra_buffer = NULL;
        memset(&header, 0, RSC_HEADER_SIZE);
    }
}


int RequestDecode(struct rsc_header * header, char * buffer){
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

int ResultEncode(struct rsc_header * header, char * read_buffer){
    header->size = RSC_HEADER_SIZE;
    switch(header->p_flag) {
        case 0: {
            memcpy(read_buffer, header, RSC_HEADER_SIZE);
            break;
        }
        case 1: {
            memcpy(read_buffer, header, RSC_HEADER_SIZE);
            break;
        }
        case 2: {
            OutputPointerEncode(header, read_buffer);
            break;
        }
        case 3: {
            OutputPointerEncode(header, read_buffer);
            break;
        }
        case 4: {
            break;
        }
    }
    return 1;
}

int OutputPointerEncode(struct rsc_header * header, char * read_buffer){
    header->size = RSC_HEADER_SIZE + header->p_count_out;

    memcpy(read_buffer, header, RSC_HEADER_SIZE);
    memcpy(read_buffer + RSC_HEADER_SIZE, header->p_addr_out, header->p_count_out);
    return 1; 
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
    fprintf(stderr, "syscall:%ld, p_flag:%d, size:%d, error:%d\n", header->syscall,header->p_flag, header->size, header->error);
    fprintf(stderr, "rax:%lld, rdi:%lld, rsi:%lld, rdx:%lld, r10:%lld, r8:%lld, r9:%lld\n", header->rax, header->rdi,header->rsi,header->rdx,header->r10,header->r8,header->r9);
    fprintf(stderr, "p_location_in:%d, p_location_out:%d, p_count_in:%d, p_count_out:%d\n", header->p_location_in, header->p_location_out, header->p_count_in, header->p_count_out);
    fprintf(stderr, "p_addr_in:%s, p_addr_out:%s\n", header->p_addr_in, header->p_addr_out);
}