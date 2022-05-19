#include "rsc_socket.h"

/* 在客户端创建一个socket连接，返回socket文件描述符 */
int client_connect_socket(char* ipaddr, int port){
    struct sockaddr_in server_addr;
    int sockfd = -1;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ipaddr);
    server_addr.sin_port = htons(port);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("[Error]: Can't create socket in client!\n[strerror]: %s\n", strerror(errno));
        exit(0);
    }

    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        printf("[Error]: Can't connect socket server! server ip: %s, server_port is: %d\n[strerror]: %s\n", ipaddr, port, strerror(errno));
        exit(0);
    }

    return sockfd;
}