#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

struct test_a {
    int a;
    int b;
};

struct test_b {
    int c;
    int d;
};

struct test_struct {
    struct test_a e;
    struct test_b f;
};

void tprintf(struct test_a * test){
    struct test_b b;
    memcpy(&b, (char *)test + sizeof(struct test_a), sizeof(struct test_b));
    printf("test output:%d\n", b.c);
}

char * tpointer(){
    char * tbuffer = NULL;
    tbuffer = (char *)malloc(sizeof(struct test_a));
    ((struct test_a *)tbuffer)->a = 1;
    ((struct test_a *)tbuffer)->b = 2;
    printf("tbuffer->a:%d, tbuffer->b:%d\n", ((struct test_a *)tbuffer)->a, ((struct test_a *)tbuffer)->b);
    return tbuffer;
}

void tpointer_1(char **tbuffer){
    *tbuffer = (char *)malloc(sizeof(struct test_a));
}

int main()
{
    // struct test_a a;
    // struct test_b b;

    // a.a = 1;
    // a.b = 2;
    // b.c = 3;
    // b.d = 4;

    // struct test_struct tstruct;
    // memset(&tstruct, 0, sizeof(tstruct));
    // memcpy(&tstruct, &a, sizeof(a));
    // memcpy((char*)&tstruct + sizeof(struct test_a), &b, sizeof(b));
    // tprintf((void *)&tstruct);

    // /* 测试子函数中malloc的内存会不会自动free */
    // struct test_a * tbuffer = NULL;
    // tbuffer = tpointer();
    // if(tbuffer == NULL){
    //     printf("flags2\n");
    // }
    // printf("tbuffer->a:%d, tbuffer->b:%d\n", tbuffer->a, tbuffer->b);
    // free(tbuffer);
    // tbuffer = NULL;

    // /* 测试，，不记得了 */
    // struct test_a * tbuffer = NULL;
    // tpointer_1((char **)&tbuffer);
    // free(tbuffer);
    // tbuffer = NULL;

    // /* 通过 RSC 测试系统调用参数用寄存器传递的对不对 */
    // char a[100] = "huomax is a shuaige!";
    // FILE *fp = NULL;
    // fp = fopen("test.txt", "w");
    // fwrite(a, sizeof(char), strlen(a), fp);
    // fclose(fp);

    /* 测试 char * 表示的地址和 usigned long long int 的转换 */
    char * a = "huomax is a shuaige!";
    printf("%s: %lld\n", a, (unsigned long long int)a);
    return 0;
}

