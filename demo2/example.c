/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int main(){
    FILE *fp = NULL;
    char ch[20] = "huomax is a shuaibi";

    if(NULL == (fp = fopen("test2.txt", "w"))){
        printf("can't open file!\n\n");
        exit(EXIT_FAILURE);
    }

    fwrite(ch, sizeof(ch), 20, fp);
    fclose(fp);
    return 0;

}