#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <syscall.h>
void exit(int status) {
  (void) syscall_1(SYS_exit, status);
}

