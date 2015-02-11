#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	
	//test printf
    /*
	char course[] = "Operating Systems";
	char semester[] = "Spring";
	int year = 2015;
	int hex = 255;
	char c = 'a';
	printf("Course\t\t\tSemester\tYear\n");
	printf("%s\t%s\t\t%d\n", course, semester, year);
	printf("Decimal - %d; Hexadecimal - %x\n", hex, hex);
	printf("The character is %c\n", c);
    */
    uint64_t a=(uint64_t)sbrk((void *)100);
    uint64_t b=(uint64_t)sbrk((void *)0);
    printf("%x\n",a);
    printf("%x\n",b);
	//test fork
	/*int pid, ppid; //id, status;
	ppid = getpid();
	pid = fork();
	if(pid == 0) {
		if(ppid == getppid())
			printf("getppid() working\n");
		printf("This is child process\n");
		return 0;
	}
	else if(pid>0) {
		printf("This is parent process\n");
	}
	else
		printf("Fork error\n");*/
		

	//test getcwd
	/*char buf[500];
	getcwd(buf,sizeof(buf)+1);
	printf(buf);
	printf("\n");*/

	//test open, read, write, dup, dup2
	/*int n, newfd = 6, dupfd = 7;
	char buf[500];
	n=open("/home/stufs1/smaguluri/warmup1/s15-w1/LICENSE",O_RDONLY);
	if(n > 0)
	{
		printf("File opened successfully\n");
		read(n, buf, sizeof(buf));
		write(1, buf, sizeof(buf));
	}
	printf("\n");

	dup2(n, newfd);
	printf("After dupping\n");
	read(newfd, buf, sizeof(buf));
	write(1, buf, sizeof(buf));
	printf("\n");

	dupfd = dup(1);
	write(dupfd, "dup working\n", 12);
	printf("\n");

	close(newfd);
	if(read(newfd, buf, sizeof(buf) < 0))
		printf("Read error\n");*/

	//test pipe
	/*int n;
	int fd[2];
	char buf[1025];
	char *data = "hello";

	pipe(fd);
	write(fd[1], data, 5);
	if ((n = read(fd[0], buf, 1024)) >= 0) {
		buf[n] = 0;	
		printf("read bytes from the pipe : ");
		printf(buf);
		printf("\n");
	}	
	else
		printf("read error\n");
	exit(0); */

	//test lseek
	/*int n;
	char buf[500];
	n=open("/home/stufs1/smaguluri/warmup1/s15-w1/LICENSE",O_RDONLY);
	read(n, buf, sizeof(buf));
	write(1, buf, sizeof(buf));
	printf("\nAfter lseek\n");
	lseek(n,1,SEEK_CUR);
	read(n, buf, sizeof(buf));
	write(1, buf, sizeof(buf));
	printf("\n");*/

	return 0;
}
