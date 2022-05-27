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
    struct rsc_header header;
    memset(&header, 0, RSC_HEADER_SIZE);

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

        // 获取系统调用的寄存器参数
        struct user_regs_struct u_regs;
        if (ptrace(PTRACE_GETREGS, pid, 0, &u_regs) == -1){
            printf("[client][ptrace]: errno, %d, strerror: %s\n", errno, strerror(errno));
            return -1;
        }

        // 对特定系统调用进行远程执行
        if (is_set(syscall_bitmap, u_regs.orig_rax)){
            // RSCQ 编组
            char * syscall_request = NULL;
            char * syscall_result = NULL;
            syscall_request = syscall_request_encode(header, u_regs);

            // 执行 RSCQ 
            if (write(sockfd, syscall_request, header->size) < 0){
                printf("[client][socket-write]: errno, %d, strerror: %s\n", errno, strerror(errno));
                return -1;
            }
            if (read(sockfd, &header, RSC_HEADER_SIZE) < 0){
                printf("[client][socket-read]: errno, %d, strerror: %s\n", errno, strerror(errno));
                return -1;
            }
            if (header->size > RSC_HEADER_SIZE){
                syscall_result = (char *)malloc(sizeof(char) * (header->size - RSC_HEADER_SIZE));
                if (read(sockfd, &syscall_result, header->size - RSC_HEADER_SIZE) < 0){
                    printf("[client][socket-read]: errno, %d, strerror: %s\n", errno, strerror(errno));
                    return -1;
                }
            }

            // RSCQ 解组
            syscall_request_decode(header, syscall_result);
            memset(&header, 0, RSC_HEADER_SIZE);
            free(syscall_request);
            free(syscall_result);
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
    
    return 0;
}
