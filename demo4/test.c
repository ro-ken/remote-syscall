#include <stdio.h>
#include <string.h>
#include <unistd.h>
struct test_struct {
    int a;
    int b;
};

void tprintf(struct test_struct * test){
    printf("test output:%d\n", test->b);
}

int main()
{
    struct test_struct tstruct;
    memset(&tstruct, 0, sizeof(tstruct));
    tstruct.b = 9;
    tprintf((char*)&tstruct);
    return 0;
}

