#ifndef _STDLIB_H
#define _STDLIB_H

#include <sys/defs.h>

int errno;

void exit(int status);
void atoi(char *numberString, int *number, int base);
void itoa(int number, char *str, int base);

// memory
typedef uint64_t size_t;
void *malloc(size_t size);
void free(void *ptr);
int brk(void *end_data_segment);
void * sbrk(uint64_t offset);
void mallocTest();

// processes
typedef uint32_t pid_t;
pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);
int execve(const char *filename, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options);
unsigned int sleep(unsigned int seconds);
unsigned int alarm(unsigned int seconds);
void yield();
void ps();
int kill(int pid);
// paths
char *getcwd(char *buf, size_t size);
int chdir(const char *path);

// files
typedef int64_t ssize_t;
enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
int open(const char *pathname, int flags);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
typedef uint64_t off_t;
off_t lseek(int fildes, off_t offset, int whence);
int close(int fd);
int pipe(int filedes[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);

// directories
#define NAME_MAX 255
struct dirent
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name [NAME_MAX+1];
};
#define DIR_BUF_SIZE  1024
void *opendir(const char *name);
struct dirent *readdir(void *dir);
int closedir(void *dir);

#endif
