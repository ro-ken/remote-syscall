/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

/* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>


int main(int argc, char **argv)
{
    if (argc <= 1){
        FATAL("too few arguments: %d", argc);
    }
    
    pid_t pid = fork();
    switch (pid) {
        /* error */
        case -1: 
            FATAL("%s", strerror(errno));
        /* child */
        case 0:{
            execvp(argv[1], argv + 1);
            FATAL("%s", strerror(errno));
        }
    }

    for(;;){
        sem_wait(tracer_attach);
        ptrace(PTRACE_ATTACH, pid, 0, 0);
        waitpid(pid, 0, 0); 
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
        sem_post(tracee_target);    // 通知tracee已经开始追踪

        while(TRUE) {               // 一段目标代码区开始追踪
            if(sem_trywait(tracee_target_exit)){            // tracer准备结束跟踪
                ptrace(PTRACE_ATTACH, atoi(argv[1], 0, 0)); // 解除跟踪
                sem_post(tracer_detach);                    // 通知tracee已经结束跟踪了
                break;
            }

            /* Enter next system call */
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
                printf("stop here: first ptrace_syscall\n");
                return -1;
            }
            if (waitpid(pid, 0, 0) == -1){
                printf("stop here: first waitpid\n");
                return -1;
            }

            /* get system call register */
            struct user_regs_struct regs;
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
                printf("stop here: get user_regs!\n");
            long syscall = regs.orig_rax;

            printf("syscall: %ld\n", syscall);

            /* Run system call and stop on exit */
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
                printf("stop here: second syscall\n");
                return -1;
            }
            if (waitpid(pid, 0, 0) == -1){
                printf("stop here: second waitpid\n");
                return -1;
            }
        }

    }

    return 0;
}






