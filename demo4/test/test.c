#include <stdio.h>
#include <string.h>

int main() {
    char buffer[10] = "test.txt";
    printf("buffer-size:%d\n", strlen(buffer));
    printf("9取余8:%d\n", 9%8);
    struct test{
        int a;
        int b;
        int c;
    };
    struct test test_struct;
    struct test * test_struct_p;
    printf("test_struct-size:%d\n", sizeof(test_struct));
    printf("test_struct_p-size:%d", sizeof(*test_struct_p));
    return 0;
}