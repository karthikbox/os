#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syscall.h>
#include <sys/syscall.h>


ssize_t write(int fd, const void *buf, size_t size)
{

    return (ssize_t)syscall_3((uint64_t)SYS_write,(uint64_t)fd,(uint64_t)buf,(uint64_t)size);

}
