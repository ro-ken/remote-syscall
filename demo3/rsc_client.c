#include "rsc.h"
#include "rsc_socket.h"
 
int main(int argc, char **argv)
{
    /* total number of paraments check */
    if (argc <= 1){
        FATAL("too few arguments: %d", argc);
    }

    /* inital remote syscall client socket */
    struct rsc_socket_client *client = NULL;
    client = (struct rsc_socket_client *)malloc(sizeof(struct rsc_socket_client));
    memset(client, 0, sizeof(struct rsc_socket_client));

    fill_socket_client(client);
    create_socket_client(client);       // Create client socket
    connect_socket_client(client);      // connect server


    /* Create and initial write message buffer */
    struct syscall_para * pbuffer = NULL;
    if ((pbuffer = (struct syscall_para *)malloc(sizeof(struct syscall_para))) == NULL){
        FATAL("malloc failure in pbuffer!\n");
    }
    memset(pbuffer, 0, sizeof(struct syscall_para));

    /* Create and initial read message buffer */
    struct syscall_return * gbuffer = NULL;
    if((gbuffer = (struct syscall_return *)malloc(sizeof(struct syscall_return))) == NULL) {
        FATAL("malloc failure in gbuffer!\n");
    }
    memset(gbuffer, 0, sizeof(struct syscall_return));

    /* Fork a child process to execute target program */
    pid_t pid = fork();

    switch (pid) {
        /* error */
        case -1: 
            FATAL("%s", strerror(errno));
        /* child */
        case 0:{
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            execvp(argv[1], argv + 1);
            FATAL("%s", strerror(errno));
        }
    }

    /* wait until child process stop */
    waitpid(pid, 0, 0);
    
    /* exit the tracee when exit tracer */
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
 
    /* loop to handle syscall of child process */
    for(;;){
        /* Enter next system call */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));

        /* Get system call arguments */
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
            FATAL("%s", strerror(errno));
        long syscall = regs.orig_rax;
        printf("now syscall: %d\n", syscall);

        /* handle specific syscall(now is getpid(), syscall number is 39) */
        if(syscall == NR_GETPID){
            int iret = 0;
            /* set remote syscall paraments */
            pbuffer->rax = regs.orig_rax;
            pbuffer->rdx = regs.rdx;
            pbuffer->rcx = regs.rcx;
            pbuffer->rdi = regs.rdi;
            pbuffer->rsi = regs.rsi;
            pbuffer->r8 = regs.r8;
            pbuffer->r9 = regs.r9;

            /* remote syscall request */
            if ((iret = (send(client->fd, (struct syscall_para *)pbuffer, sizeof(struct syscall_para), 0))) < 0)
            {
                perror("perror: ");
                printf("RSC: client write error!\n");
                break;
            }
            printf("iret:%d\n", iret);
            /* get remote syscall result */
            if (((iret = recv(client->fd, (struct syscall_return *)gbuffer, sizeof(struct syscall_return), 0))) < 0)
            {
                perror("perror: ");
                printf("RSC: client read error!\n");
                break;
            }
            /* continue local syscall exxcute, but syscall number redirect to a nonexistent syscall */
            regs.orig_rax = RSC_REDIRECT_SYSCALL;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1){
                fputs(" = ?\n", stderr);
                if (errno == ESRCH)
                    exit(regs.rdi);
                FATAL("%s", strerror(errno));
            }
        
            /* Restart child process, execute syscall, but will stop on syscall-exit-stop point */ 
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
            if (waitpid(pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
            
            regs.rax = gbuffer->rax;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1){
                fputs(" = ?\n", stderr);
                if (errno == ESRCH)
                    exit(regs.rdi);
                FATAL("%s", strerror(errno));
            }
            continue;
        }
        
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
    }
    
    close(client->fd);
    free(client);
    return 0;
}