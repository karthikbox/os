#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <syscall.h>

char *getcwd(char *buf, size_t size) {
	uint64_t l;
    l = syscall_2(SYS_getcwd, (uint64_t) buf, (uint64_t) size);

    if((uint64_t *)l!=NULL){
    	printf("getcwd->%s\n",buf);
		return buf;
    }
	else
		return NULL;
}

int chdir(const char *path) {
	int ret= (int) syscall_1(SYS_chdir, (uint64_t) path);
	if(ret<0){
		errno=-ret;
		return -1;
	}
	return ret;
}
