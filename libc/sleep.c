#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syscall.h>



unsigned int sleep(unsigned int seconds){
//    return (unsigned int)syscall_2()
    struct timespec req;
    req.tv_sec=seconds;
    req.tv_nsec=seconds*1000000000L;
    struct timespec rem;
    return (unsigned int)syscall_2(SYS_nanosleep,(uint64_t)&req,(uint64_t)&rem);
}
