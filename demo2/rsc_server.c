#include "rsc.h"
#include "rsc_socket.h"

int main()
{
    /* inital socket */
    printf("flag1\n");
    struct rsc_socket_server *server = NULL;
    server = (struct rsc_socket_server *)malloc(sizeof(struct rsc_socket_server));
    if (server == NULL){
            perror("perror: ");
            FATAL("server will exit!\n");
    }
    memset(server, 0, sizeof(struct rsc_socket_server));

    fill_socket_server(server);
    create_socket_server(server);
    bind_socket_server(server);
    listen_socket_server(server);
    /* blocked accpet */
    accept_socket_server(server);

    /* loop handle client message */
    int iret;
    for(;;){
        if ( (iret = write(server->client_fd, (struct syscall_para *)&buf, sizeof(struct syscall_para)) < 0)){
            perror("perror: ");
            printf("server will exit!\n");
            break;
        }
    }

    close(server->client_fd);
    close(server->fd);
    free(server);
    
    return 0;

}