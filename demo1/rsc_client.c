/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Linux */
#include <sys/socket.h>

#define SERVER_IP "120.48.30.70"
#define SERVER_PORT 1234
#define MAX_CONNECTS 5

#define FATAL(...) \
    do { \
        fprintf(stderr, "RSC: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

int main(){
    /* Create socket */
    int client_fd = -1;
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        FATAL("Create socket failure!\n");
    }

    /* Create connect */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    serv_addr.sin_port = htons(SERVER_PORT);
    connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
   
    /* receive return message */
    for(;;){
            char buffer[100];
            memset(&buffer, 0, sizeof(buffer));
            read(sock, buffer, sizeof(buffer)-1);
            printf("Read message: %s", buffer);
            sleep(1);
    }
    

    close(client_fd);
    return 0;
}