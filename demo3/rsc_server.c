#include "rsc.h"
#include "rsc_socket.h"

int main()
{
    /* inital rsc_socket_server struct */
    fprintf(stderr, "server pid: %d\n", getpid());
    fprintf(stderr, "RSC: initial socket ...\n");
    struct rsc_socket_server *server = NULL;
    server = (struct rsc_socket_server *)malloc(sizeof(struct rsc_socket_server));
    if (server == NULL){
            perror("perror: ");
            FATAL("socket server struct malloc failure, server will exit!\n");
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
    if ((gbuffer = (struct syscall_para *)malloc(sizeof(struct syscall_para))) == NULL){
        FATAL("RSC: malloc failure in gbuffer!\n");
    }
    memset(gbuffer, 0, sizeof(struct syscall_para));

    struct syscall_return * pbuffer = NULL;
    if ((pbuffer = (struct syscall_return *)malloc(sizeof(struct syscall_return))) == NULL) {
        FATAL("RSC: malloc failure in pbuffer!\n");
    }
    memset(pbuffer, 0, sizeof(struct syscall_return));

    /* loop handle client message */
    int iret=0;
    for(;;){
        if ((iret = recv(server->client_fd, (struct syscall_para *)gbuffer, 112, 0)) < 0){
            perror("perror: ");
            fprintf(stderr, "RSC: server read error!\n");
            break;
        }
        printf("iret:%d\n", iret);
        fprintf(stderr, "syscall:%ld (RDI: %ld, RSI: %ld, RDX: %ld, RCX: %ld, R8: %ld, R9: %ld)\n",
                (long)gbuffer->rax,
                (long)gbuffer->rdi, (long)gbuffer->rsi, (long)gbuffer->rdx,
                (long)gbuffer->rcx, (long)gbuffer->r8,  (long)gbuffer->r9);
        
        /* remote syscall */
        pbuffer->rax = syscall(gbuffer->rax, gbuffer->rdi, gbuffer->rsi, gbuffer->rdx, gbuffer->rcx, gbuffer->r8, gbuffer->r9);

        /* get remote syscall result to write buffer */
        pbuffer->enumber = errno;
        strcpy(pbuffer->ebuffer, strerror(errno));

        /* return remote syscall result */
        if ((iret = send(server->client_fd, (struct syscall_return *)pbuffer, sizeof(struct syscall_return), 0) < 0)){
            perror("perror: ");
            fprintf(stderr, "RSC: server write error!\n");
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