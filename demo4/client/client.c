#include "../include/rsc_include.h"
#include "../include/rsc_include_client.h"

// 已实现的系统调用位示图, 目前已实现：0, 1, 2, 3, 257
unsigned long long int syscall_bitmap[9] = {15, 0, 0, 0, 3, 0, 0, 0, 0};

int main(int argc, char **argv)
{
    // 参数不满足则结束程序
    if (argc <= 1) FATAL("[%s][%s]: too few arguments!","client", "arguments");

    // 创建子进程，使用子进程去执行目标程序(注意需要在子进程中使用PTRACE_SYSCALL)
    pid_t pid = fork();
    switch (pid) {
        /* error */
        case -1: 
            FATAL("[%s][%s]: fork failure! %s","client", "fork", strerror(errno));
        /* child */
        case 0:{
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            execvp(argv[3], argv + 3);
            FATAL("[%s][%s]: fork failure! %s","client", "fork", strerror(errno));
        }
    }

    // 创建远程系统调用请求头
    struct rsc_header header;
    memset(&header, 0, RSC_HEADER_SIZE);
    char * syscall_request = NULL;
    char * syscall_result = NULL;


    // socket connect
    int sockfd = -1;
    if ((sockfd = socket_connect_client(argv[1], atoi(argv[2]))) < 0) FATAL("[%s][%s]: socket connect failure!", "client", "socket");

    // 当子进程使用PTRACE_TRACEME之后,会陷入暂停态
    waitpid(pid, 0, 0);

    // 当父进程死了之后，强制杀死子进程
    ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

    for(;;){
        // syscall-enter_stop
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) ERROR("[%s][%s]: in syscall-enter-stop!\n", "client", "ptrace_syscall");
        if (waitpid(pid, 0, 0) == -1) ERROR("[%s][%s]: in syscall-enter-stop!\n", "client", "waitpid");

        // 获取系统调用的寄存器参数
        struct user_regs_struct regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) ERROR("[%s][%s]: in syscall-enter-stop!\n", "client", "ptrace_getregs");

        // 判断是否要执行远程系统调用
        if (is_set(syscall_bitmap, regs.orig_rax)){
            // 远程系统调用请求(rscq, remote syscall request) 编组
            syscall_request = syscall_request_encode(&header, &regs);

            // 向 server 发送 rscq
            if (write(sockfd, syscall_request, header.size) < 0) ERROR("[%s][%s]: in write remote syscall request!\n", "client", "write");

            // 接收 server 的远程系统调用结果(rscr, remote syscall result)
            if (read(sockfd, &header, RSC_HEADER_SIZE) < 0) ERROR("[%s][%s]: in read remote syscall result header!\n", "client", "read");
            if (header.size > RSC_HEADER_SIZE){
                syscall_result = (char *)malloc(sizeof(char) * (header.size - RSC_HEADER_SIZE));
                if (read(sockfd, syscall_result, header.size - RSC_HEADER_SIZE) < 0) ERROR("[%s][%s]: in read remote syscall result buffer!\n", "client", "read");
            }

            // 解组远程系统调用请求执行结果
            if (syscall_return_decode(&regs, &header, syscall_result) < 0) ERROR("[%s][%s]: in syscall-enter-stop!\n", "client", "return_decode");

            // 系统调用重定向, 使其指向一个不会被执行的系统调用号
            regs.rax = RSC_REDIRECT_SYSCALL;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) ERROR("[%s][%s]: in syscall-enter-stop!\n", "client", "ptrace_setregs");
            
            // syscall-exit-stop
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) ERROR("[%s][%s]: in syscall-exit-stop!\n", "client", "ptrace_syscall");
            if (waitpid(pid, 0, 0) == -1) ERROR("[%s][%s]: in syscall-exit-stop!\n", "client", "waitpid");

            // 将远程系统调用的结果返回给本地进程
            regs.rax = header.rax;
            if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) ERROR("[%s][%s]: in syscall-enter-stop!\n", "client", "ptrace_setregs");

            // Handling the crime scene
            memset(&header, 0, RSC_HEADER_SIZE);
            free(syscall_request);
            free(syscall_result);
            syscall_request = NULL;
            syscall_result = NULL;

            continue;
        }


        // syscall-exit-stop
        if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) ERROR("[%s][%s]: in syscall-exit-stop!\n", "client", "ptrace_syscall");
        if (waitpid(pid, 0, 0) == -1) ERROR("[%s][%s]: in syscall-exit-stop!\n", "client", "waitpid");


    }
    
    return 0;
}
