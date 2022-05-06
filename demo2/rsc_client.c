#include "rsc.h"
#include "rsc_socket.h"

int main(int argc, char **argv)
{
    /* parament number check */
    if (arg <= 1){
        FATAL("too few arguments: %d", argc);
    }

    /* inital rscsocket */
    struct rsc_socket_client *client = NULL;
    client = (struct rsc_socket_client *)malloc(sizeof(struct rsc_socket_client));
    memset(client, 0, sizeof(struct rsc_socket_client));

    fill_socket_client(client);
    create_socket_client(client);
    connect_socket_client(client);


    /* create write message buffer */
    struct syscall_para * pbuffer = NULL;
    pbuffer = (struct syscall_para *)malloc(sizeof(struct syscall_para));
    memset(pbuffer, 0, sizeof(struct syscall_para));

    /* create read message buffer */
    struct syscall_return * gbuffer = NULL;
    gbuffer = (struct syscall_return *)malloc(sizeof(struct syscall_return));
    memset(gbuffer, 0, sizeof(struct syscall_return));

    /* remote syscall */
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

    /* parent */
    waitpid(pid, 0, 0);
    
    /* exit the tracee when exit tracer */
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
 
    /* receive return message */
    for(;;){
        /* Enter next system call */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));

        /* Gather system call arguments */
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
            FATAL("%s", strerror(errno));
        long syscall = regs.orig_rax;

        /* Run system call and stop on exit */
        if(syscall == NR_GETPID){
            /* set remote syscall paraments */
            pbuffer->rax = regs.orig_rax;
            pbuffer->rdx = regs.rdx;
            pbuffer->rcx = regs.rcx;
            pbuffer->rdi = regs.rdi;
            pbuffer->rsi = regs.rsi;
            pbuffer->r8 = regs.r8;
            pbuffer->r9 = regs.r9;

            /* remote syscall apply */
            if ((write(client->fd, (struct syscall_para *)&pbuffer, sizeof(struct syscall_para))) < 0)
            {
                perror("perror: ");
                printf("client will exit!\n");
                break;
            }

            /* get remote syscall result */
            if ((read(client->fd, (struct syscall_return *)&gbuffer, sizeof(struct syscall_return))) < 0)
            {
                perror("perror: ");
                printf("client will exit!\n");
                break;
            }

            regs.orig_rax = RSC_REDIRECT_SYSCALL;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1){
                fputs(" = ?\n", stderr);
                if (errno == ESRCH)
                    exit(regs.rdi);
                FATAL("%s", strerror(errno));
            }
        
            /* restart child process, exec syscall, but will stop on syscall-exit-stop point */ 
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