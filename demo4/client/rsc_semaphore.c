#include "../include/rsc.h"
#include "../include/rsc_semaphore.h"

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
int SemaphorePost(int sem_id, int sem_num, int sem_flg){
    struct sembuf sem_buffer;
    sem_buffer.sem_num = sem_num;
    sem_buffer.sem_op = 1;
    sem_buffer.sem_flg = sem_flg;
    return semop(sem_id, &sem_buffer, 1);
}

// wait 操作
int SemaphoreWait(int sem_id, int sem_num, int sem_flg){
    struct sembuf sem_buffer;
    sem_buffer.sem_num = sem_num;
    sem_buffer.sem_op = -1;
    sem_buffer.sem_flg = sem_flg;
    return semop(sem_id, &sem_buffer, 1);
}

// 获得信号量 key, 如果该信号量不存在则创建并初始化所有值为0
int SemaphoreGet(){
    int sem_id = -1;
    union SemUnion arg;
    unsigned short array[4] = {0};

    if ((sem_id = semget(CreateKey(SEM_PATHNAME), 4, IPC_CREAT | IPC_EXCL | 0666)) >= 0){
        arg.array = array;
        if (semctl(sem_id, 0, SETALL, arg) < 0){
            printf("initial signal error!\n");
            return -1;
        }
    }
    else if (errno == EEXIST)
    {
        sem_id = semget(CreateKey(SEM_PATHNAME), 1, 0666);
    }
    else {
        printf("Create signal error!\n");
        return -1;
    }
    return sem_id;
}

int SemaPhoreDestroy(int sem_id){
    return semctl(sem_id, 0, IPC_RMID);
}