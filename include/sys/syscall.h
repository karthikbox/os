#ifndef __SYS_SYSCALL_H
#define __SYS_SYSCALL_H
#include <sys/defs.h>

struct proc;

#define SYS_exit       60
#define SYS_brk        12
#define SYS_fork       57
#define SYS_getpid     39
#define SYS_getppid   110
#define SYS_execve     59
#define SYS_wait4      61
#define SYS_nanosleep  35
#define SYS_alarm      37
#define SYS_getcwd     79
#define SYS_chdir      80
#define SYS_open        2
#define SYS_read        0
#define SYS_write       1
#define SYS_lseek       8
#define SYS_close       3
#define SYS_pipe       22
#define SYS_dup        32
#define SYS_dup2       33
#define SYS_getdents   78
#define SYS_yield      24

#define T_SYSCALL 128

struct timespec{
    time_t tv_sec;//seconds
    time_t tv_nsec;
};

typedef uint32_t pid_t;

void init_syscall();
void do_yield();
void do_fork();
void handle_pf();
size_t do_write(int fd,const void *bf,size_t len);
void do_brk(void *end_data_segment);
void do_exit(int status,struct proc *p);
pid_t do_getpid();
pid_t do_getppid();
void do_nanosleep(struct timespec *req,struct timespec *rem);
void do_waitpid(pid_t pid, int *status, int options);
void do_execve(char *filename, char* argv[], char* envp[]);
void do_read(int fd, void *buf, size_t count);
void do_pipe(int * fd_arr);
void do_close(int fd);
int do_dup(int old_fd);
int do_dup2(int old_fd,int new_fd);
char * do_getcwd(char *buf,size_t size);
int do_chdir(const char *path);
#endif
