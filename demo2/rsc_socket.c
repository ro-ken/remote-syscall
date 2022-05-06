#include "rsc_socket.h"
#include "rsc.h"

/* initial sockaddr_in struct via ip, port defined in rsc.h*/
void fill_socket_client(struct rsc_socket_client * client)
{
    memset(&client->server_addr, 0, sizeof(struct sockaddr_in));

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_addr.s_addr = inet_addr(RSC_SERVER_IP);
    /* Note: here must use htons for set port, can't use htonl */
    client->server_addr.sin_port = htons(RSC_SERVER_PORT);
}

/* create client socket, use tcp */
void create_socket_client(struct rsc_socket_client * client)
{
    if ((client->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("[error]:");
        FATAL("[error]: Create socket failure!\n");
    }
}

/* connect server socket */
void connect_socket_client(struct rsc_socket_client * client)
{
    int client_connect = -1;
    if ((client_connect = connect(client->fd, (struct sockaddr *)&client->server_addr, (socklen_t)sizeof(struct sockaddr)) < 0)) {
        perror("error");
        FATAL("connect server failure!\n");
    }
}

/* initial sockaddr_in struct via ip, port defined in rsc.h*/
/* note that server ip is INADDR_ANY means server bind anything ip address-> It's the choice of most people */
void fill_socket_server(struct rsc_socket_server * server)
{
    memset(&server->server_addr, 0, sizeof(struct sockaddr));

    server->server_addr.sin_family = AF_INET;
    server->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server->server_addr.sin_port = htons(RSC_SERVER_PORT);

    memset(&server->client_addr, 0, sizeof(struct sockaddr));
}

/* create socket, server_fd is file descriptor of tcp */
void create_socket_server(struct rsc_socket_server * server)
{
    if ((server->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0 ){
        perror("[error]:");
        FATAL("[error]: Create socket failure!\n");
    }
}

/* bind server addr */
void bind_socket_server(struct rsc_socket_server * server)
{
    int server_bind = -1;
    if ((server_bind = bind(server->fd, (struct sockaddr *)&server->server_addr, (socklen_t)sizeof(struct sockaddr))) < 0) {
        perror("[error]:");
        FATAL("[error]: bind socket failure!\n");
    }
}

/* listen socket */
void listen_socket_server(struct rsc_socket_server * server)
{
    int server_listen = -1;
    if ((server_listen = listen(server->fd, RSC_MAX_CONNECTS)) < 0 ){
        perror("[error]:");
        FATAL("[error]: Listen failure!\n");
    }
}

/* accpet socket */
void accept_socket_server(struct rsc_socket_server * server)
{
    socklen_t addrlen = sizeof(struct sockaddr);
    if ((server->client_fd = accept(server->fd, (struct sockaddr *)&server->server_addr, &addrlen)) < 0) {
        perror("[error]:");
        FATAL("[error]: Accept failure!\n");
    }
}
