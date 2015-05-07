#include<stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("exec1 says hi\n");
	int fd=open("mnt/test.txt",O_RDONLY);
	printf("fd->%d\n",fd);
	char buf[1000];
	int a;
	int pid=fork();
	if(pid>0){
		/* parent */
		printf("parent says hi\n");
		a=read(fd,buf,10);
		buf[a]='\0';
		printf("read %d bytes\n",a);
		printf("%s\n",buf);
		
		a=read(fd,buf,10);
		buf[a]='\0';
		printf("read %d bytes\n",a);
		printf("%s\n",buf);

		sleep(3);
		
		a=read(fd,buf,10);
		buf[a]='\0';
		printf("read %d bytes\n",a);
		printf("%s\n",buf);
		waitpid(pid,0,0);
	}
	else if(pid==0){
		/* child */
		printf("child says hi\n");
			a=read(fd,buf,10);
			buf[a]='\0';	
			printf("read %d bytes\n",a);
			printf("%s\n",buf);
			
			
			
			close(fd);
			
			a=read(fd,buf,5);
			buf[a]='\0';	
			printf("read %d bytes\n",a);
			printf("%s\n",buf);
			return 1;
	}
	else{
		printf("fork failed\n");
	}

	
	return 1;
}
