/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

/* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
/* tcp socket paraments */

#define RSC_SERVER_IP "192.168.16.4"
#define RSC_CLIENT_IP "120.48.30.70"
#define RSC_SERVER_PORT 40000
#define RSC_MAX_CONNECTS 5

int main()
{
    int server_fd = 0, client_fd = 0;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(RSC_CLIENT_IP);
    /* Note: here must use htons for set port, can't use htonl */
    server_addr.sin_port = htons(RSC_SERVER_PORT);

    if ((client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("[error]");
    }

    if (connect(client_fd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(struct sockaddr) < 0)) {
        perror("error");
    }

    char buffer[10] = {"wangguoku"};
    while(1){
        write(client_fd, buffer, sizeof(char)*10);
    }
    return 0;
}