/* C Standard Library */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>

/* POSIX */
#include <unistd.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>

/* Linux */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

static sem_t tracer_attach;
static sem_t tracer_detach;
static sem_t tracee_target;
static sem_t tracee_traget_exit;



void signal_init(){
    sem_init(&tracee_target, 1, 0);
    sem_init(&tracee_target, 1, 0);
    sem_init(&tracee_target, 1, 0);
    sem_init(&tracee_target, 1, 0);
}

void signal_destory(){
    sem_destroy(&tracee_target);
    sem_destroy(&tracee_traget_exit);
    sem_destroy(&tracer_attach);
    sem_destroy(&tracer_detach);
}

int create_sem_set(int nums);
int get_sem_set(int nums);
int init_sem_set(int sem_id, int which, int val);
int P(int sem_id);
int V(int sem_id);
int destroy(int sem_id);