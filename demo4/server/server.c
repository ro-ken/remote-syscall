#include "../include/rsc.h"
#include "../include/rsc_server.h"

int main(int argc, char **argv)
{
    if (argc <= 1){
        FATAL("[%s][%s]: too few parameters!\n", "server", "parameters");
    }

    int sockfd = InitialSocket(atoi(argv[2]),argv[1]);
    struct sockaddr_in client;
    socklen_t len = sizeof(struct sockaddr_in);

    for(;;){
        int sockfd_c = -1;
        if (accept(sockfd_c, (struct sockaddr*)&client, &len) < 0)
            FATAL("[%s][%s]: socket accept failure!, %s", "server", "accept", strerror(errno));

        pid_t pid = fork();
        switch (pid) {
            case -1: 
                FATAL("[%s][%s]: fork failure! %s","client", "fork", strerror(errno));
            case 0:{
                if(fork() > 0) exit(0);
                if (RSCHandle(sockfd_c) < 0) exit(-1);
            }
        }
        close(sockfd_c);
    }

    close(sockfd);
    return 0;
}


