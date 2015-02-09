#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syscall.h>
#include <sys/syscall.h>

struct timespec{
    time_t tv_sec;//seconds
    long tv_nsec;
};

unsigned int sleep(unsigned int seconds){
//    return (unsigned int)syscall_2()
    struct timespec req={seconds,seconds*1000000000L};
    struct timespec rem;
    return (unsigned int)syscall(&req,&rem);
}
