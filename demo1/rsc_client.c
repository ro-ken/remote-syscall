/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#define SERVER_IP "120.48.30.70"
#define SERVER_PORT 1234
#define MAX_CONNECTS 5

struct syscall_para {
    unsigned long long int rax;
    unsigned long long int rdi;
    unsigned long long int rsi;
    unsigned long long int rdx;
    unsigned long long int rcx;
    unsigned long long int r8;
    unsigned long long int r9;
};

#define FATAL(...) \
    do { \
        fprintf(stderr, "RSC: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

int main()
{
    /* char buffer for test communication */
    char buffer[19];

    /* Create socket */
    int client_fd = -1;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        FATAL("Create socket failure!\n");
    }

    /* Create connect */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htonl(SERVER_PORT);
    connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
   
    /* receive return message */
    for(;;){
            memset(&buffer, 0, sizeof(buffer));
            read(client_fd, buffer, sizeof(buffer));
            printf("Read message: %s", buffer);
    }
    
    close(client_fd);
    return 0;
}