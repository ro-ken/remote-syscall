#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

int main()
{
    struct test_a a;
    struct test_b b;

    a.a = 1;
    a.b = 2;
    b.c = 3;
    b.d = 4;

    struct test_struct tstruct;
    memset(&tstruct, 0, sizeof(tstruct));
    memcpy(&tstruct, &a, sizeof(a));
    memcpy((char*)&tstruct + sizeof(struct test_a), &b, sizeof(b));
    tprintf((void *)&tstruct);
    return 0;
}

