#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <syscall.h>

char *getcwd(char *buf, size_t size) {
	long l;
    l = syscall_2(SYS_getcwd, (uint64_t) buf, (uint64_t) size);

    //while(buf[i] != '\0')
    //   	i++;
   
    if(l>0){
    	//printf("getcwd worked\n");
	return buf;
    }
    else if(l < 0){
    	//printf("getcwd error\n");
	return NULL;
    }
    return NULL;
}

int chdir(const char *path) {
	return (int) syscall_1(SYS_chdir, (uint64_t) path);
}
