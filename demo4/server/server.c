#include "../include/rsc_include.h"
#include "../include/rsc_include_server.h"

int main(int argc, char **argv)
{
    if (argc <= 1){
        FATAL("too few arguments: %d", argc);
    }

    int sockfd = initial_socket(atoi(argv[2]),argv[1]);
    //用来接收客户端的socket地址结构体
    struct sockaddr_in client;
    socklen_t len = sizeof(struct sockaddr_in);

    while(1)
    {
        int sockfd_c = -1;
        accept(sockfd, (struct sockaddr*)&client, &len);
        if (accept(sockfd, (struct sockaddr*)&client, &len) < 0)
        {
            printf("[server][accept]: errno, %d, strerror: %s, first waitpid\n", errno, strerror(errno));
            return -1;
        }

        //每次建立一个连接后fork出一个子进程进行收发数据
        pid_t pid = fork();
        switch (pid) {
            /* error */
            case -1: 
                FATAL("%s", strerror(errno));
            /* child */
            case 0:{
                if(fork() > 0) exit(0);

                //close(listen_sock);
                char * buffer = NULL;
                char * syscall_result = NULL;
                struct rsc_header header;
                memset(&header, 0, RSC_HEADER_SIZE);

                while(1)
                {
                    // get rscq(remote syscall request)
                    if(read(sockfd_c, &header, RSC_HEADER_SIZE) < 0){
                        printf("[server][read]: errno, %d, strerror: %s, first waitpid\n", errno, strerror(errno));
                        return -1;
                    }
                    if (header.size > RSC_HEADER_SIZE){
                        buffer = (char *)malloc(sizeof(char) * (header.size - RSC_HEADER_SIZE));
                        if(read(sockfd_c, buffer, header.size - RSC_HEADER_SIZE) < 0){
                            printf("[server][read]: errno, %d, strerror: %s, first waitpid\n", errno, strerror(errno));
                            return -1;
                        }
                    }

                    // rscq(remote syscall request) decode
                    if (syscall_request_decode(&header, buffer) < 0){
                            printf("[server][decode]: error, will exit!\n");
                            return -1;
                    }

                    // execute rscq
                    if (syscall_request_execute(&header) < 0){
                            printf("[server][execute]: error, will exit!\n");
                            return -1;
                    }

                    // rscq result encode
                    if (syscall_result_encode(&header) < 0){
                            printf("[server][execute]: error, will exit!\n");
                            return -1;
                    }
                    // return rscq result
                }
            }
        }

        close(sock);
        // while(waitpid(-1, NULL, WNOHANG) > 0);
    }
    return 0;
}
