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

int main(){
    sem_post(tracer_attach);        // 通知tracer准备开始追踪
    sem_wait(tracee_target);        // 阻塞等待追踪进程开始追踪

    FILE *fp = NULL;
    char * buffer = "huomax is a big shuaige!";
    fp = open("test_s.txt", "w");
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
    fclose();

    sem_post(tracee_target_exit);   // 通知tracer准备结束跟踪
    sem_wait(tracer_detach);        // 阻塞等待追踪进程结束追踪
    sem_destory()
    return 0;
}