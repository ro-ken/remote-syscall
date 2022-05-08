/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>

/* Linux */
#include <syscall.h>
#include <sys/ptrace.h>

#define FATAL(...) \
    do { \
        if (errno == ESRCH) { \
            printf("[remote syscall tip]: tracee is eixted!\n"); \
            exit(EXIT_FAILURE); \
        } \
        fprintf(stderr, "[remote syscall fatal]: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0).

#define WARNING(...) \
    do { \
        fprintf(stdout, "[remote syscall warning]: " __VA_ARGS__); \
        fputc('\n', stdout); \
    }while(0)

int main(int argc, char **argv)
{
    if (argc <= 1)
        FATAL("too few arguments: %d", argc);

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
    printf("parrent pid is: %d, child pid is %d\n", getpid(), pid);

    /* exit the tracee when exit tracer */
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

    for (;;) {
        /* Enter next system call */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            printf("stop here: first ptrace_syscall\n");
            FATAL("%s", strerror(errno));
        }

        if (waitpid(pid, 0, 0) == -1){
            printf("stop here: first waitpid\n");
            FATAL("%s", strerror(errno));
        }


        /* Gather system call arguments */
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1){
            printf("stop here: first getregs\n");
            FATAL("%s", strerror(errno));
        }
        long syscall = regs.orig_rax;

        /* Run system call and stop on exit */
        if(syscall == 39){
            regs.orig_rax = 10000;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1){
                fputs(" = ?\n", stderr);
                if (errno == ESRCH)
                    exit(regs.rdi);
                FATAL("%s", strerror(errno));
            }

            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
            if (waitpid(pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
            
            regs.rax = getpid();
            regs.orig_rax = 39;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1){
                fputs(" = ?\n", stderr);
                if (errno == ESRCH)
                    exit(regs.rdi);
                FATAL("%s", strerror(errno));
            }

            continue;
        }

        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            printf("stop here: second ptrace_syscall\n");
            FATAL("%s", strerror(errno));
        }

        if (waitpid(pid, 0, 0) == -1){
            printf("stop here: second waitpid\n");
            FATAL("%s", strerror(errno));
        }
        
    }
}

/*

{
    SYSCALL         # - restart tracee, tracee running until the next system call
    waitpid         # - tracee enter syscall-enter-stop point, tracer run
}

{
    SYSCALL         # - restart tracee, tracee enter the system call
    waitpid         # - tracee enter syscall-exit-stop point, tracer run
}

*/