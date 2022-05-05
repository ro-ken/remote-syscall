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
    char testbuf[] = "huomax is shuaige!";

    /* create socket, server_fd is file descriptor of tcp */
    int server_fd = -1;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        FATAL("Create socket failure!\n");
    }

    /* bind server addr */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if ((server_addr.sin_addr.s_addr = inet_addr(SERVER_IP)) < 0 ){
        FATAL("Addr transform failure!\n");
    }
    server_addr.sin_port = htonl(SERVER_PORT);
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    /* listen socket */
    int server_listen = -1;
    if ((server_listen = listen(server_fd, MAX_CONNECTS)) < 0 ){
        FATAL("Listen failure!\n");
    }

    /* Create client addr */
    struct sockaddr_in client_addr;
    int socklen = sizeof(struct sockaddr_in);
    memset(&client_addr, 0, sizeof(client_addr));
    int client_fd = -1;
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&socklen)) < 0){
        FATAL("Accept failure!\n");
    }

    /* loop handle client message */
    for(;;){
        write(client_fd, testbuf, sizeof(testbuf));
        sleep(2);
    }

    close(client_fd);
    close(server_fd);
    
    return 0;



    // /* create connect */
    // int socket_tcp_connect = -1;
    // if ((socket_tcp_connect = connect(socket_tcp_fd, (struct sockaddr*)server_addr, sizeof(server_addr)))){
    //     FATAL("Create connect failure!\n");
    // }





}