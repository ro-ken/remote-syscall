/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int main(){
    FILE *fp = NULL;
    char ch[100] = "huomax is a shuaibi";

    if(NULL == (fp = fopen("testsdfsdf.txt", "w"))){
        printf("can't open file!\n\n");
        exit(EXIT_FAILURE);
    }

    fwrite(ch, sizeof(ch), 100, fp);
    fclose(fp);
    return 0;

}