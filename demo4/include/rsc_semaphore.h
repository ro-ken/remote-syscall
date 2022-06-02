#ifndef RSC_SEMAPHORE_H_
#define RSC_SEMAPHORE_H_  3

/* semaphore */
#include <sys/ipc.h>
#include <sys/sem.h>

/* Process synchronization semaphore */
#define ATTACH  0
#define DETACH  1
#define TARGET  2
#define TARGET_EXIT  3

/* semaphore key (use ftok) */
#define SEM_PATHNAME "./system_v_yaoshi.txt"

/* system v semaphore buffer for semctl() */
union SemUnion {          
    int                 value;
    struct semid_ds *   buffer;
    unsigned short *    array;
};

// semaphore abstract
int     CreateKey(const char * pathName);
int     SemaphorePost(int semid, int sem_num, int sem_flg);
int     SemaphoreWait(int semid, int sem_num, int sem_flg);
int     SemaphoreGet();
int     SemaPhoreDestroy(int semId);

#endif