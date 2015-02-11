

int main(int argc, char* argv[], char* envp[]);  
void exit(int status); -- code done and working

// memory
typedef uint64_t size_t;
void *malloc(size_t size);
void free(void *ptr);
int brk(void *end_data_segment);

// processes
typedef uint32_t pid_t;
pid_t fork(void); -- code done and working
pid_t getpid(void); -- code done and working
pid_t getppid(void); -- code done and working
int execve(const char *filename, char *const argv[], char *const envp[]);
pid_t waitpid(pid_t pid, int *status, int options); -- not working
unsigned int sleep(unsigned int seconds); -- not working
unsigned int alarm(unsigned int seconds);

// paths
char *getcwd(char *buf, size_t size); -- code done and working
int chdir(const char *path); -- code done and working

// files
typedef int64_t ssize_t;
enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
int open(const char *pathname, int flags); -- code done and working
ssize_t read(int fd, void *buf, size_t count); -- code done and working
ssize_t write(int fd, const void *buf, size_t count); -- code done and working
enum { SEEK_SET = 0, SEEK_CUR = 1, SEEK_END = 2 };
typedef uint64_t off_t;
off_t lseek(int fildes, off_t offset, int whence); -- code done and working
int close(int fd); -- code done and working
int pipe(int filedes[2]); -- code done and working
int dup(int oldfd); -- code done and working
int dup2(int oldfd, int newfd); -- code done and working

// directories
#define NAME_MAX 255
struct dirent
{
	long d_ino;
	off_t d_off;
	unsigned short d_reclen;
	char d_name [NAME_MAX+1];
};
void *opendir(const char *name);
struct dirent *readdir(void *dir);
int closedir(void *dir);

// stdio
printf(const char*, ...);  -- code done for %d, %x, %s and working; should check for %c
scanf(const char*, ...)
