#include "rsc.h"
#include "rsc_socket.h"

int main()
{
    /* inital rsc_socket_server struct */
    fprintf(stderr, "RSC: initial socket ...\n");
    struct rsc_socket_server *server = NULL;
    server = (struct rsc_socket_server *)malloc(sizeof(struct rsc_socket_server));
    if (server == NULL){
            perror("perror: ");
            FATAL("server will exit!\n");
    }
    memset(server, 0, sizeof(struct rsc_socket_server));

    /* start socket server */
    fill_socket_server(server);
    create_socket_server(server);
    bind_socket_server(server);
    listen_socket_server(server);

    /* blocked wait client connect */
    accept_socket_server(server);

    /* create read/write message buffer */
    struct syscall_para * gbuffer = NULL;
    gbuffer = (struct syscall_para *)malloc(sizeof(struct syscall_para));
    memset(gbuffer, 0, sizeof(struct syscall_para));

    struct syscall_return * pbuffer = NULL;
    pbuffer = (struct syscall_return *)malloc(sizeof(struct syscall_return));
    memset(pbuffer, 0, sizeof(struct syscall_return));

    /* loop handle client message */
    int iret;
    for(;;){
        if ((iret = read(server->client_fd, (struct syscall_para *)&gbuffer, sizeof(struct syscall_para)) < 0)){
            perror("perror: ");
            printf("server will exit!\n");
            break;
        }

        fprintf(stderr, "syscall:%ld (RDI: %ld, RSI: %ld, RDX: %ld, RCX: %ld, R8: %ld, R9: %ld)\n",
                buffer->rax,
                (long)buffer->rdi, (long)buffer->rsi, (long)buffer->rdx,
                (long)buffer->rcx, (long)buffer->r8,  (long)buffer->r9);
        
        /* remote syscall */
        pbuffer->rax = syscall(buffer->rax, buffer->rdi, buffer->rsi, buffer->rdx, buffer->rcx, buffer->r8, buffer->r9);

        /* get remote syscall result to write buffer */
        pbuffer->errno_num = errno;
        char * errno_info = NULL;
        errno_info = strerror(errno);
        strcpy(pbuffer->errno_info, errno_info);

        /* return remote syscall result */
        if ((iret = write(server->client_fd, (struct syscall_return *)&pbuffer, sizeof(struct syscall_return)) < 0)){
            perror("perror: ");
            printf("server will exit!\n");
            break;
        }
    }

    close(server->client_fd);
    close(server->fd);
    
    free(pbuffer);
    free(gbuffer);
    free(server);
    
    return 0;

}