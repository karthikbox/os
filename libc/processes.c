#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <syscall.h>

pid_t fork(void) {
	return  (pid_t) syscall_0(SYS_fork);
}

pid_t getpid(void) {
	return  (pid_t) syscall_0(SYS_getpid);
}

pid_t getppid(void) {
	return  (pid_t) syscall_0(SYS_getppid);
}

pid_t waitpid(pid_t pid, int *status, int options) {
	return (pid_t) syscall_3(SYS_wait4, (uint64_t) pid, (uint64_t) status, (uint64_t) options);
}

unsigned int sleep(unsigned int seconds) {

	unsigned int nanoseconds = seconds * 1000000000;
	return  (unsigned int) syscall_1(SYS_nanosleep, (uint64_t) nanoseconds); 
}

unsigned int alarm(unsigned int seconds) {
	return  (unsigned int) syscall_1(SYS_alarm, (uint64_t) seconds); 
}