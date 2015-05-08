#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/syscall.h>



int open(const char *pathname, int flags) {
	int ret= (int) syscall_2(SYS_open, (uint64_t) pathname, (uint64_t) flags);
	if(ret<0){
		errno=-ret;
		return -1;
	}
	else{
		return ret;
	}
}

ssize_t read(int fd, void *buf, size_t count) {
	ssize_t ret= (ssize_t) syscall_3(SYS_read, (uint64_t) fd, (uint64_t) buf, (uint64_t) count);
	if(ret<0){
		errno=-ret;
		return -1;
	}
	else{
		return ret;
	}
	
}

ssize_t write(int fd, const void *buf, size_t count) { 
	
 	ssize_t ret= (ssize_t) syscall_3(SYS_write, (uint64_t) fd, (uint64_t) buf, (uint64_t) count);
	if(ret < 0){
		errno = -ret;
		return -1;
	}
	else{
		return ret;
	}

}

off_t lseek(int fd, off_t offset, int whence) {
	off_t ret= (off_t) syscall_3(SYS_lseek, (uint64_t) fd, (uint64_t) offset, (uint64_t) whence);
	if(ret < 0){
		/* ret is less than zero */
		errno=-(ssize_t)ret;
		return -1;
	}
	else{
		/* ret is >= 0 */
		return ret;
	}
}

int close(int fd) {
	return (int) syscall_1(SYS_close, (uint64_t) fd);
}

int pipe(int filedes[2]) {
	return (int) syscall_1(SYS_pipe, (uint64_t) filedes);
}

int dup(int oldfd) {
	return (int) syscall_1(SYS_dup, (uint64_t) oldfd);
}

int dup2(int oldfd, int newfd) {
	return (int) syscall_2(SYS_dup2, (uint64_t) oldfd, (uint64_t) newfd);
}

