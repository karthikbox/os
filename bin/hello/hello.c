#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	//int pid;
	//test printf
	//printf("Hello World!\n");
	//test fork
	/*pid = fork();
	if(pid == 0)
		printf("This is child process\n");
	else if(pid>0)
		printf("This is parent process\n");
	else
		printf("Fork error\n");
		*/

	//test getcwd
	/*char buf[500];
	getcwd(buf,sizeof(buf)+1);
	printf(buf);
	printf("\n");*/
	//test open
	int n, newfd = 6, dupfd = 7;
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
		printf("Read error\n");
	return 0;
}
