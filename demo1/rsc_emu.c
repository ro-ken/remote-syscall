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
    long targetsyscall = -1;
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
        if(targetsyscall == 1){
            if (waitpid(pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
                targetsyscall = -1;
        }
        else{
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
            if (waitpid(pid, 0, 0) == -1)
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
        
        /* when syscall is write, SYSEMU skip syscall and enter next loop */
        if(syscall == 1){
            targetsyscall = 1;
            if (ptrace(PTRACE_SYSEMU, pid, 0, 0) == -1)
                FATAL("%s", strerror(errno));
            continue;
        }

        /* other syscall */
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

/*

{
    SYSCALL         # - restart tracee, tracee running until the next system call
    waitpid         # - tracee enter syscall-enter-stop point, tracer run
}

{
    SYSCALL         # - restart tracee, tracee enter the system call
    waitpid         # - tracee enter syscall-exit-stop point, tracer run
}

--------------------------------------------------------------------------------

save_target = -1;
{
    if(save_target = target){
        waitpid         # - tracee enter syscall-enter-stop point, tracer run
        save_target = -1;
    }
    else{
        SYSCALL         # - restart tracee, tracee running until the next system call
        waitpid         # - tracee enter syscall-enter-stop point, tracer run
    }

}


if(orign_eax == target){
    save_target = target;
    SYSEMU          # - restart tracee but non-execution, and have not syscall-exit-stop point
}
else{
    SYSCALL         # - restart tracee, tracee running until the next system call
    waitpid         # - tracee enter syscall-enter-stop point, tracer run
}

note: 按照ptrace手册上说，PTRACE_EMU有两点需要注意：
    1. PTRACE_SYSEMU同PTRACE_SYSCALL一样会重启tracee，但PTRACE_SYSEMU会不执行系统调用
    2. PTRACE_SYSEMU没有syscall-exit-stoped
那么为什么我上面的程序哪怕使用了PTRACE_SYSEMU，实际还是会执行系统调用？
是我翻译错了？还是我程序的逻辑错了？
先跳过这个方法，有时间去看看源码怎么实现PTRACE_SYSEMU

*/