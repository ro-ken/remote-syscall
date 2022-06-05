#include "../include/rsc.h"
#include "../include/rsc_client.h"
#include "../include/rsc_semaphore.h"

unsigned long long syscall_bitmap[9] = {15, 0, 0, 0, 2, 0, 0, 0, 0};

int main(int argc, char **argv)
{
    // parameters check
    if (argc <= 1) FATAL("[%s][%s]: too few parameters!","client", "parameters");

    // Create child process to exec target program
    pid_t pid = fork();
    switch (pid) {
        case -1: 
            FATAL("[%s][%s]: fork failure! %s","client", "fork", strerror(errno));
        case 0:{
            execvp(argv[3], argv + 3);
            FATAL("[%s][%s]: child execvp failure! %s","client", "execvp", strerror(errno));
        }
    }

    // Create struct rsc_header to manage remote syscall request encode and decode
    struct rsc_header header;
    struct user_regs_struct regs;
    memset(&header, 0, RSC_HEADER_SIZE);
    memset(&regs, 0, sizeof(struct user_regs_struct));
    char * extra_buffer = NULL;

    // Create and initial semaphore
    int sem_id;
    union SemUnion sem_arg;
    struct sembuf sem_buffer;
    if ((sem_id = SemaphoreGet()) < 0) FATAL("[%s][%s]: get semaphore failure!","client", "SemaphoreGet");

    // socket connect
    int sockfd = -1;
    if ((sockfd = SocketConnect(argv[1], atoi(argv[2]))) < 0) FATAL("[%s][%s]: socket connect failure!", "client", "socket");

    for(;;){

        // block waiting child process request tracing
        SemaphoreWait(sem_id, ATTACH, 0);
        ptrace(PTRACE_ATTACH, pid, 0, 0);
        waitpid(pid, 0, 0);
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);
        SemaphorePost(sem_id, TARGET, 0);

        // tracing syscall to remote execute
        for(;;){

            // detach tracing when child process leave target code area
            if (SemaphoreWait(sem_id, TARGET_EXIT, IPC_NOWAIT) != -1){
                ptrace(PTRACE_DETACH, pid, 0, 0);
                SemaphorePost(sem_id, DETACH, 0);
            }

            // syscall-enter-stop
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1){
                if (errno == 3) {
                    printf("[%s][%s]: target code area exit\n", "client", "ptrace_syscall");
                    break;
                }
                printf("[%s][%s]: in syscall-enter-stop, %d, %s\n", "client", "ptrace_syscall", errno, strerror(errno));
                break;
            }
            if (waitpid(pid, 0, 0) == -1) 
                FATAL("[%s][%s]: in syscall-enter-stop, %d, %s\n", "client", "waitpid", errno, strerror(errno));
            
            // get syscall register
            if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) 
                FATAL("[%s][%s]: in syscall-enter-stop, %d, %s\n", "client", "ptrace_getregs", errno, strerror(errno));
            
            // whether the syscall to be executed remotely
            if (IsSet(syscall_bitmap, regs.orig_rax) == 1){

                // remote syscall request encode
                char write_buffer[1000];
                memset(write_buffer, 0, 1000);
                RequestEncode(&regs, &header, write_buffer, pid);

                // socket write remote syscall request
                int ret = 0;
                if ((ret = write(sockfd, write_buffer, 1000)) < 0) 
                    FATAL("[%s][%s]: in syscall-enter-stop, %d, %s, write bytes: %d\n", "client", "write", errno, strerror(errno), ret);
                
                // Debug check struct rsc_header
                struct rsc_header t_header;
                memcpy(&t_header, write_buffer, RSC_HEADER_SIZE);
                DebugPrintf(&t_header);

                // socket read remote syscall execute result
                memset(write_buffer, 0, 1000);
                if (( ret = read(sockfd, write_buffer, 1000)) < 0) 
                    FATAL("[%s][%s]: in syscall-enter-stop, %d, %s, read bytes: %d\n", "client", "read", errno, strerror(errno), ret);
                memcpy(&header, write_buffer, RSC_HEADER_SIZE);
                if (header.size > RSC_HEADER_SIZE){
                    extra_buffer = (char *)malloc(sizeof(char) * (header.size - RSC_HEADER_SIZE));
                    memcpy(extra_buffer, write_buffer+RSC_HEADER_SIZE, header.size - RSC_HEADER_SIZE);
                }

                // remote syscall result decode
                ResultDecode(&regs, &header, extra_buffer, pid);

                // redirect syscall 
                regs.orig_rax = RSC_REDIRECT_SYSCALL;
                if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) 
                    FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "ptrace_setregs", errno, strerror(errno));

                
                // syscall-exit-stop
                if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) 
                    FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "ptrace_syscall", errno, strerror(errno));
                if (waitpid(pid, 0, 0) == -1) 
                    FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "waitpid", errno, strerror(errno));
                
                // return remote syscall execute to child process
                if (ptrace(PTRACE_GETREGS, pid, 0, &regs) == -1) 
                    FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "ptrace_setregs", errno, strerror(errno));
                regs.rax = header.rax;
                if (ptrace(PTRACE_SETREGS, pid, 0, &regs) == -1) 
                    FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "ptrace_setregs", errno, strerror(errno));

                // Handling the crime scene
                memset(&header, 0, RSC_HEADER_SIZE);
                if (extra_buffer != NULL){
                    free(extra_buffer);
                    extra_buffer = NULL;
                }
                memset(&regs, 0, sizeof(struct user_regs_struct));

                continue;
            }

            // syscall-exit-stop
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) == -1) 
                FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "ptrace_syscall", errno, strerror(errno));
            if (waitpid(pid, 0, 0) == -1) FATAL("[%s][%s]: in syscall-exit-stop, %d, %s\n", "client", "waitpid", errno, strerror(errno));
        }
    }
    SemaPhoreDestroy(sem_id);
    return 0;
}
