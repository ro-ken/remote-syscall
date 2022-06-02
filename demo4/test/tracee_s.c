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
#define SEM_PATHNAME "./system_v_yaoshi.txt"

// 通过 ftok 获取信号量钥匙号
int CreateKey(const char * pathName)
{
    FILE *fd = NULL;
 
    if ((fd = fopen( pathName,"r")) == NULL){
        printf("Open file error!\n");
        return -1;
    }
 
    fclose(fd);
    return ftok(pathName, 0);
}

// post 操作
int sem_post(struct sembuf * sem_buffer, int semid, int sem_num, int sem_flg){
    *sem_buffer.sem_num = sem_num;
    *sem_buffer.sem_op = 1;
    *sem_buffer.sem_flg = sem_flg;
    return semop(semid, sem_buffer, 1);
}

// wait 操作
int sem_wait(struct sembuf * sem_buffer, int semid, int sem_num, int sem_flg){
    *sem_buffer.sem_num = sem_num;
    *sem_buffer.sem_op = -1;
    *sem_buffer.sem_flg = sem_flg;
    return semop(semid, sem_buffer, 1);
}

// 获得信号量 key, 如果该信号量不存在则创建并初始化所有值为0
int sem_get(){
    int semId = -1;
    union semun arg;
    unsigned short array[4] = {0};

    if ((semId = semget(CreateKey(SEM_PATHNAME), 4, IPC_CREAT | IPC_EXCL | 0666)) >= 0){
        arg.array = array;
        if (semctl(semId, 0, SETALL, arg) < 0){
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
    return semId;
}

int sem_destroy(int semId){
    return semctl(semId, 0, IPC_RMID);
}

int main(){
    int semId;
    union semun arg;
    unsigned short array[4] = {0};
    struct sembuf sem_buffer;
    sem_buffer.sem_num = ATTACH;
    sem_buffer.sem_op = 1;
    sem_buffer.sem_flg = 0;
    
    semId = semget(CreateKey(SEM_PATHNAME), 1, 0666);

    semop(semId, &sem_buffer, 1);           // 通知 tracer 准备开始追踪

    sem_buffer.sem_num = TARGET;
    sem_buffer.sem_op = -1;
    semop(semId, &sem_buffer, 1);           // 阻塞等待 tracer 开始追踪

    FILE *fp = NULL;
    char * buffer = "huomax is a big shuaige!";
    fp = fopen("test_s.txt", "w");
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
    fclose(fp);

    sem_buffer.sem_num = TARGET_EXIT;
    sem_buffer.sem_op = 1;
    semop(semId, &sem_buffer, 1);           // 通知tracer准备结束跟踪

    sem_buffer.sem_num = DETACH;
    sem_buffer.sem_op = -1;
    semop(semId, &sem_buffer, 1);           // 阻塞等待追踪进程结束追踪
    printf("远程系统调用代码区结束了, 我的使命也结束了!\n");
    return 0;
}