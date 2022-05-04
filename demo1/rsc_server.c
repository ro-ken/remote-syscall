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


int main()
{
    char testbuf[] = "huomax is shuaige!";
    /* create socket, socket_tcp_fd is file descriptor of tcp */
    int socket_tcp_fd = -1;
    if ((socket_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        FATAL("Create socket failure!\n");
    }

    /* bind server addr */
    struct sockaddr_in server_addr;
    memset(&server_addr, 09, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_post = htons(SERVER_PORT);
    bind(socket_tcp_fd, (struct sockaddr_in*)&server_addr, sizeof(server_addr));

    /* listen socket */
    int socket_tcp_listen = -1;
    if ((socket_tcp_listen = listen(socket_tcp_fd, MAX_CONNECTS)) < 0 ){
        FATAL("Listen failure!\n");
    }

    for(;;){
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int client_fd = -1;
        if ((client_fd = accept(socket_tcp_fd, (struct sockaddr*)&client_addr, sizeof(client_addr))) < 0){
            FATAL("Accept failure!\n");
        }
        sleep(3);
        write(client_fd, testbuf, sizeof(testbuf));
    }

    close(client_fd);
    close(socket_tcp_fd);
    
    return 0;



    // /* create connect */
    // int socket_tcp_connect = -1;
    // if ((socket_tcp_connect = connect(socket_tcp_fd, (struct sockaddr*)server_addr, sizeof(server_addr)))){
    //     FATAL("Create connect failure!\n");
    // }





}