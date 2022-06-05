#include <stdio.h>
#include <string.h>
#include "../include/rsc_semaphore.h"

int main() {
    int sem_id;
    sem_id = SemaphoreGet();
    SemaphorePost(sem_id, ATTACH, 0);           // 通知 tracer 准备开始追踪
    SemaphoreWait(sem_id, TARGET, 0);           // 阻塞等待 tracer 开始追踪

    // // 测试 sys_openat, sys_write, sys_close能否远程执行
    // FILE *fp = fopen("ce_w.txt", "w");                  // sys_open, 服务端打开 ce_w.txt文件 
    // char * buffer = "huomax is a big shuaige!";         
    // fwrite(buffer, sizeof(char), strlen(buffer), fp);   // sys_read, 服务端将字符串写入到 ce_w.txt文件
    // fclose(fp);                                         // sys_close，服务端关闭 ce_w.txt文件

    // 测试 sys_openat, sys_read, sys_write, sys_close能否远程执行
    FILE *fp_r = fopen("cs_r.txt", "r");        // sys_open, 服务端打开ce_r.txt文件 
    char buffer_r[100];                         
    fread(buffer_r, sizeof(char), 100, fp_r);   // sys_read, 服务端从ce_r.txt文件中读取100个字节数据
    fclose(fp_r);                               // sys_close, 服务端关闭打开的ce_r.txt文件
    printf("read data: %s\n", buffer_r);        // sys_rite, 服务端会显示该字符串

    SemaphorePost(sem_id, TARGET_EXIT, 0); // 通知tracer准备结束跟踪
    SemaphoreWait(sem_id, DETACH, 0);      // 阻塞等待追踪进程结束追踪
    printf("远程系统调用代码区结束了, 我的使命也结束了!\n");
    return 0;
}