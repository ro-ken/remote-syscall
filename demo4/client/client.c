#include "include/rsc_include.h"

unsigned long long int syscall_bitmap[9] = {15, 0, 0, 0, 1, 0, 0, 0, 0};

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
            execvp(argv[3], argv + 3);
            FATAL("%s", strerror(errno));
        }
    }

    // 等待子进程因为PTRACE_SYSCALL而第一次暂停
    waitpid(pid, 0, 0);

    // 当子进程结束时，结束父进程
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

    for(;;){
        
    }
    //连接成功进行收数据
    char buf[1024];
    while(1)
    {
        printf("send###");
        fflush(stdout);

        ssize_t _s = read(0, buf, sizeof(buf)-1);
        buf[_s] = 0;
        write(sock, buf, _s);
    }
    //close(sock);

    return 0;
}
