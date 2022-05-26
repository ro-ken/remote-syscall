#include "../include/rsc_include.h"
#include "../include/rsc_include_client.h"

// 已实现的系统调用位示图, 目前已实现：0, 1, 2, 3, 257
unsigned long long int syscall_bitmap[9] = {15, 0, 0, 0, 3, 0, 0, 0, 0};

int main(int argc, char **argv)
{
    if (argc <= 1){
        FATAL("too few arguments: %d", argc);
    }

    // 创建远程系统调用请求的相关数据结构
    struct rsc_regs regs;
    memset(&regs, 0, RSC_REGS_SIZE);
    struct rsc_header header;
    memset(&header, 0, RSC_HEADER_SIZE);
    char *syscall_request = NULL;

    // 创建client端的socket文件描述符并与server端建立连接
    int sockfd = -1;
    if ((sockfd = client_connect_socket(argv[1], atoi(argv[2]))) < 0){
        exit(EXIT_FAILURE);
    }

    // 创建子进程，使用子进程去执行目标程序(注意需要在子进程中使用PTRACE_SYSCALL)
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

    // 等待子进程因为PTRACE_SYSCALL而第一次暂停
    waitpid(pid, 0, 0);

    // 父死，子随
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

    for(;;){
        // syscall-enter_stop
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            printf("[client][ptrace]: errno, %d, strerror: %s, first syscall\n", errno, strerror(errno));
            return -1;
        }
        if (waitpid(pid, 0, 0) == -1){
            printf("[client][ptrace]: errno, %d, strerror: %s, first waitpid\n", errno, strerror(errno));
            return -1;
        }

        struct user_regs_struct u_regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &u_regs) == -1){
            printf("[client][ptrace]: errno, %d, strerror: %s\n", errno, strerror(errno));
            return -1;
        }
        
        // syscall-exit_stop
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
            printf("[client][ptrace]: errno, %d, strerror: %s\n", errno, strerror(errno));
            return -1;
        }
        if (waitpid(pid, 0, 0) == -1){
            printf("[client][ptrace]: errno, %d, strerror: %s\n", errno, strerror(errno));
            return -1;
        }

    }
    // //连接成功进行收数据
    // char buf[1024];
    // while(1)
    // {
    //     printf("send###");
    //     fflush(stdout);

    //     ssize_t _s = read(0, buf, sizeof(buf)-1);
    //     buf[_s] = 0;
    //     write(sock, buf, _s);
    // }
    // //close(sock);

    return 0;
}
