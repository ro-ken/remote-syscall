#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
    FILE * fp = NULL;
    char gbuffer[100];
    memset(gbuffer, 0, sizeof(char)*100);
    
    fp = fopen("test.txt","r");
    strcpy(gbuffer, strerror(errno));
    printf("errno: %d, errno_info: %s", errno, gbuffer);
    return 0;
}