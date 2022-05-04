/* C standard library */
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/ptrace.h>
#include <syscall.h>

#define MAX_SYSCALL_NUMBER = 1000