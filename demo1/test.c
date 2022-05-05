/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/ptrace.h>
#include <syscall.h>

#define MAX_SYSCALL_NUMBER 1000

extern char *syscall_table[MAX_SYSCALL_NUMBER];
int test();

// 加载系统调用映射表
int load_syscall_table(){

    FILE *fp = NULL;
    char buff[200];
    if(NULL == (fp = fopen("syscall_table.txt","r"))){
        printf("can't open file!");
        exit(1);
    }

    fgets(buff, 20, fp);
    fclose(fp);
    printf("输出看看：%s", buff);
    return 0;

}


int main(){
    // load_syscall_table();
    char testbuf[] = "huomax is shuaige!";
    printf("%d\n", sizeof(testbuf));
    int test_return = -1;
    char buf[100] = "huomax is shuaige";
    if ((test_return = test()) == 2){
        printf("error\n");
        
    }
    printf("the char shuzu sizeof is: %ld\n", sizeof(buf));
    return 0;

}

int test(){
    return 2;
}