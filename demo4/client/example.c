#include <stdio.h>
#include <string.h>
#include "../include/rsc_semaphore.h"

int main() {
    int sem_id;
    sem_id = SemaphoreGet();
    SemaphorePost(sem_id, ATTACH, 0);           // 通知 tracer 准备开始追踪
    SemaphoreWait(sem_id, TARGET, 0);           // 阻塞等待 tracer 开始追踪

    FILE *fp = fopen("./test_s.txt", "r");
    // char * buffer = "huomax is a big shuaige!";
    // fwrite(buffer, sizeof(char), strlen(buffer), fp);
    fclose(fp);


    SemaphorePost(sem_id, TARGET_EXIT, 0);           // 通知tracer准备结束跟踪
    SemaphoreWait(sem_id, DETACH, 0);           // 阻塞等待追踪进程结束追踪
    printf("远程系统调用代码区结束了, 我的使命也结束了!\n");
    return 0;
}