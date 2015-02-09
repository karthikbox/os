#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <syscall.h>

char *getcwd(char *buf, size_t size) {
	long l;
	int i = 0;
    l = syscall_2(SYS_getcwd, (uint64_t) buf, (uint64_t) size);

    while(buf[i] != '\0')
       	i++;
   
    if(i == l-1)
    	printf("getcwd worked\n");
    else if(l == -1)
    	printf("getcwd error\n");
    else
    	printf("l is not -1\n");

    return buf;
}

int chdir(const char *path) {
	return (int) syscall_1(SYS_chdir, (uint64_t) path);
}