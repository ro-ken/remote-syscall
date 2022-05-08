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
        fprintf(stderr, "RSC: " __VA_ARGS__); \
        fputc('\n', stderr); \
        exit(EXIT_FAILURE); \
    } while (0)

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
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1)
            FATAL("%s", strerror(errno));
        long syscall = regs.orig_rax;

        /* Print a representation of the system call */
        fprintf(stderr, "enter syscall:%ld (RDI: %ld, RSI: %ld, RDX: %ld, R10: %ld, R8: %ld, R9: %ld)\n",
                syscall,
                (long)regs.rdi, (long)regs.rsi, (long)regs.rdx,
                (long)regs.r10, (long)regs.r8,  (long)regs.r9);

        /* Run system call and stop on exit */
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        if (waitpid(pid, 0, 0) == -1)
            FATAL("%s", strerror(errno));
        
        struct user_regs_struct return_regs;
        /* Get system call result */
        if (ptrace(PTRACE_GETREGS, pid, 0, &return_regs) == -1) {
            fputs(" = ?\n", stderr);
            if (errno == ESRCH)
                exit(regs.rdi); // system call was _exit(2) or similar
            FATAL("%s", strerror(errno));
        }

        /* Print a representation of the system call */
        fprintf(stderr, "exit syscall:%ld (RDI: %ld, RSI: %ld, RDX: %ld, R10: %ld, R8: %ld, R9: %ld)",
                syscall,
                (long)return_regs.rdi, (long)return_regs.rsi, (long)return_regs.rdx,
                (long)return_regs.r10, (long)return_regs.r8,  (long)return_regs.r9);

        /* Print system call result */
        fprintf(stderr, " = %ld\n", (long)return_regs.rax);
    }
}


/*存在的一些问题
 * 1. 通过openat系统调用打开不存的文件时返回在rax中的是-2, 而不是-1, 这是为什么？

*/