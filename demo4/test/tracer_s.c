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
#include <sys/ipc.h>
#include <sys/sem.h>

union semun {          
    int                 val;
    struct semid_ds *   buf;
    unsigned short *    array;
};

#define ATTACH  0
#define DETACH  1
#define TARGET  2
#define TARGET_EXIT  3

// 通过 ftok 获取信号量钥匙号
int CreateKey(const char * pathName)
{
    FILE *fd = NULL;
    ;
 
    if ((fd = fopen( pathName,"r")) == NULL)
    {
        printf("Open file error!\n");
        return -1;
    }
 
    fclose(fd);
    return ftok(pathName, 0);
}
#define SEM_PATHNAME "./system_v_yaoshi.txt"

int main(int argc, char **argv)
{
    if (argc <= 1){
        printf("too few arguments\n");
        exit(-1);
    }
   int semId;
   union semun arg;
   unsigned short array[4] = {0,0,0,0};
   struct sembuf sem_buffer;
   sem_buffer.sem_num = ATTACH;
   sem_buffer.sem_op = -1;
   sem_buffer.sem_flg = 0;

    // 创建和初始化信号量集
    if ((semId = semget(CreateKey(SEM_PATHNAME), 4, IPC_CREAT | IPC_EXCL | 0666)) >= 0){
        arg.array = array;
        if (semctl(semId, 0, SETALL, arg) < 0)
        {
            printf("initial signal error!\n");
            return -1;
        }
    }
    else if (errno == EEXIST)
    {
        semId = semget(CreateKey(SEM_PATHNAME), 1, 0666);
    }
    else {
        printf("Create signal error!\n");
        return -1;
    }
    
    pid_t pid = fork();
    switch (pid) {
        /* error */
        case -1: 
            printf("fork error!\n");
            exit(-1);
        /* child */
        case 0:{
            execvp(argv[1], argv + 1);
            printf("fork error!\n");
            exit(-1);
        }
    }

    for(;;){
        printf("attach:%d\n",semctl(semId, ATTACH, GETVAL));
        semop(semId, &sem_buffer, 1);       // 阻塞等待 tracee请求被跟踪

        ptrace(PTRACE_ATTACH, pid, 0, 0);
        waitpid(pid, 0, 0); 
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

        sem_buffer.sem_num = TARGET;
        sem_buffer.sem_op = 1;
        semop(semId, &sem_buffer, 1);           // 通知 tracee 已经开始追踪

        sem_buffer.sem_op = -1;
        sem_buffer.sem_num = TARGET_EXIT;
        sem_buffer.sem_flg = IPC_NOWAIT;      // 非阻塞同步

        while(1) {               // 一段目标代码区开始追踪
            // printf("flag-target_exit-enter-tracer: %d\n", semctl(semId, TARGET_EXIT, GETVAL));
            if(semop(semId, &sem_buffer, 1) != -1){                   // 询问tracer是否准备结束跟踪
                ptrace(PTRACE_DETACH, pid, 0, 0); // 解除跟踪
                sem_buffer.sem_num = DETACH;
                sem_buffer.sem_op = 1;
                sem_buffer.sem_flg = 0;
                semop(semId, &sem_buffer, 1);                    // 通知 tracee 已经结束跟踪了
                // printf("flag-target_exit-exit-tracer: %d\n", semctl(semId, DETACH, GETVAL));
            }

            /* Enter next system call */
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
                printf("stop here: first ptrace_syscall,%s, %d\n",strerror(errno), semctl(semId, ATTACH, GETVAL));
                sem_buffer.sem_num = ATTACH;
                sem_buffer.sem_op = -1;
                sem_buffer.sem_flg = 0;
                break;
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






