Path functionality
command our sbush supports: set PATH <give a path>
case1: set PATH $PATH:/usr/bin/abc -> this command appends /usr/bin/abc to the existing path
case2: set PATH /usr/bin/abc:$PATH -> this command prepends /usr/bin/abc to the existing path
case3: set PATH /usr/bin/abc -> this command removes the existing path and sets to /usr/bin/abc

PS1 functionality
command our sbush supports: set PS1 "<give a word>"
example: set PS1 "sbush$ " -> this command changes the prompt to sbush$ 
example: set PS1 sbush$ -> this command throws an error

Pipe functionality
command our sbush supports: <command1> [args]|<command2> [args][|<command3> [args]] (No leading spaces/trailing spaces
example: ls|head|tail
example: /bin/ls|head|tail
example: ls -al|head -3|tail -1

EXECUTE BINARY functionality
syntax 1:
<filepath>
our shell first tries to execute this file as is, if it fails, it tries to look in the PATH directories for file.
example: sbush rootfs/bin/sbush -> executes the binary 
example: sbush ls -> tries to execute the command as is, it fails if there is no file named ls in the current directory, then it looks for in all the PATH directories

EXECUTE SCRIPT functionality
syntax:
sbush <filepath>
example: sbush /rootfs/test.dat -> executes the commands in the file test.dat.
NOTE: file CAN BE ANY extension.
Interpreter working for scripts.
Include the following line as the first line of the script and provide executable permissions to the script
#! ./rootfs/bin/sbush
example: ./script.sh -> Can be run from outside our shell as well
example: script.sh -> Our shell can also run without ./ before the name of the script.


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
