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


int main(){
    int server_fd = 0, client_fd = 0;
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("[error]");
    }
    server_addr.sin_family = AF_INET;
    // server->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_addr.s_addr = inet_addr(RSC_SERVER_IP);
    server_addr.sin_port = htons(RSC_SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, (socklen_t)sizeof(struct sockaddr)) < 0) {
        perror("[error]");
    }

    if (listen(server_fd, RSC_MAX_CONNECTS) < 0 ){
        perror("[error]:");
    }
    
    socklen_t addrlen = sizeof(struct sockaddr);
    if (client_fd = accept(server_fd, (struct sockaddr *)&server_addr, &addrlen) < 0) {
        perror("[error]:");
    }

    char buffer[10];
    memset(buffer, 0, sizeof(char)*10);
    while(1){
        read(client_fd,buffer, sizeof(char)*10);
        printf("read:%s\n", buffer);
    }
}
