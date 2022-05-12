#ifndef _RSC_SOCKET_H
#define _RSC_SOCKET_H 1

#include "rsc.h"

/* tcp socket paraments */
#define RSC_SERVER_IP "120.48.30.70"
#define RSC_SERVER_PORT 10000
#define RSC_MAX_CONNECTS 5

/* socket server struct */
struct rsc_socket_server {
    int fd;
    int client_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
};

/* socket client struct */
struct rsc_socket_client {
    int fd;
    struct sockaddr_in server_addr;
};


/* socket client function pack  */
void fill_socket_client(struct rsc_socket_client * client);
void create_socket_client(struct rsc_socket_client * client);
void connect_socket_client(struct rsc_socket_client * client);

/* socket server function pack  */
void fill_socket_server(struct rsc_socket_server * server);
void create_socket_server(struct rsc_socket_server * server);
void bind_socket_server(struct rsc_socket_server * server);
void listen_socket_server(struct rsc_socket_server * server);
void accept_socket_server(struct rsc_socket_server * server);

#endif