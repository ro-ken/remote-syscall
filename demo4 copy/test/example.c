#include <stdio.h>
#include <string.h>

int main() {
    FILE *fp = fopen("./test_s.txt", "w");
    char * buffer = "huomax is a big shuaige!";
    fwrite(buffer, sizeof(char), strlen(buffer), fp);
    fclose(fp);
}