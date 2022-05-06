#include "rsc.h"
#include "rsc_socket.h"

int main()
{
    /* char buffer for test communication */
    struct syscall_para buf;

    /* inital socket */
    struct rsc_socket_client *client = NULL;
    client = (struct rsc_socket_client *)malloc(sizeof(struct rsc_socket_client));
    memset(client, 0, sizeof(struct rsc_socket_client));

    fill_socket_client(client);
    printf("family:%d, a_addr:%d, port: %d", client->server_addr.sin_family, client->server_addr.sin_addr.s_addr, client->server_addr.sin_port);
    create_socket_client(client);
    printf("fd:%d", client->fd);
    connect_socket_client(client);
 
    /* receive return message */
    for(;;){
        memset(&buf, 0, sizeof(struct syscall_para));
        if ((read(client->fd, (struct syscall_para *)&buf, sizeof(struct syscall_para))) < 0)
        {
            perror("perror: ");
            printf("client will exit!\n");
            break;
        }
        printf("syscall para: %lld, %lld, %lld, %lld, %lld, %lld, %lld\n", buf.rax, buf.rdi, \
            buf.rsi, buf.rdx, buf.rcx, buf.r8, buf.r9
        );
    }
    
    close(client->fd);
    free(client);
    return 0;
}