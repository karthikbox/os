#include<stdio.h>
#include <stdlib.h>
#include<string.h>
int main(int argc, char* argv[], char* envp[]) {
	printf("exec1\n");
	int i;
	for(i=0;argv[i];i++){
		printf("argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("envp[%d]->%s\n",i,envp[i]);
	}
	strcat(envp[1],":/bin");
	int ret=execve("/bin/exec1",argv,envp);
	if(ret==-1){
		printf("execve failed\n");
		printf("errno->%d\n",errno);
	}
	/* printf("exec1 says hi\n"); */
	/* argv[0]=NULL; */
	/* if(execve("/mnt/test.txt",argv,envp) == -1){ */
	/* 	printf("exece failed. errno->%d",errno); */
	/* } */
	/* int fd=open("mnt/test.txt",O_RDONLY); */
	/* if(fd ==-1){ */
	/* 	printf("file open failed\n"); */
	/* 	exit(0); */
	/* } */
	/* printf("fd->%d\n",fd); */
	/* int p[2]; */
	/* pipe(p); */
	/* dup2(fd,STDIN); */
	/* char buf[1000]; */
	/* int a; */
	/* int pid =100; */
	/* if(pid>0){ */
	/* 	/\* parent *\/ */
	/* 	printf("parent says hi\n"); */
	/* 	scanf("%s",buf); */
	/* 	printf("%s\n",buf); */
	/* 	scanf("%s",buf); */
	/* 	printf("%s\n",buf); */

	/* 	/\* a=read(fd,buf,10); *\/ */
	/* 	/\* buf[a]='\0'; *\/ */
	/* 	/\* printf("errno %d bytes\n",errno); *\/ */
	/* 	/\* printf("read %d bytes\n",a); *\/ */
	/* 	/\* printf("%s\n",buf); *\/ */
		
	/* 	/\* a=read(fd,buf,10); *\/ */
	/* 	/\* buf[a]='\0'; *\/ */
	/* 	/\* printf("read %d bytes\n",a); *\/ */
	/* 	/\* printf("%s\n",buf); *\/ */

	/* 	/\* a=read(fd,buf,20); *\/ */
	/* 	/\* buf[a]='\0'; *\/ */
	/* 	/\* printf("read %d bytes\n",a); *\/ */
	/* 	/\* printf("%s\n",buf); *\/ */


	/* 	/\* printf("###########\n"); *\/ */
	/* 	/\* int fd1=open("mnt/test.txt",O_RDONLY); *\/ */
	/* 	/\* if(fd1 ==-1){ *\/ */
	/* 	/\* 	printf("file open failed\n"); *\/ */
	/* 	/\* 	exit(0); *\/ */
	/* 	/\* } *\/ */
	/* 	/\* printf("fd->%d\n",fd1); *\/ */

	/* 	/\* printf("parent says hi\n"); *\/ */
	/* 	/\* a=read(fd1,buf,10); *\/ */
	/* 	/\* buf[a]='\0'; *\/ */
	/* 	/\* printf("errno %d bytes\n",errno); *\/ */
	/* 	/\* printf("read %d bytes\n",a); *\/ */
	/* 	/\* printf("%s\n",buf); *\/ */
		
	/* 	/\* a=read(fd1,buf,10); *\/ */
	/* 	/\* buf[a]='\0'; *\/ */
	/* 	/\* printf("read %d bytes\n",a); *\/ */
	/* 	/\* printf("%s\n",buf); *\/ */

	/* 	/\* a=read(fd1,buf,20); *\/ */
	/* 	/\* buf[a]='\0'; *\/ */
	/* 	/\* printf("read %d bytes\n",a); *\/ */
	/* 	/\* printf("%s\n",buf);		 *\/ */
		
		
	/* } */
	/* else if(pid==0){ */
	/* 	/\* child *\/ */
	/* 	printf("child says hi\n"); */
	/* 		a=read(fd,buf,10); */
	/* 		buf[a]='\0';	 */
	/* 		printf("read %d bytes\n",a); */
	/* 		printf("%s\n",buf); */
			
			
			
	/* 		close(fd); */
			
	/* 		a=read(fd,buf,5); */
	/* 		buf[a]='\0';	 */
	/* 		printf("read %d bytes\n",a); */
	/* 		printf("%s\n",buf); */
	/* 		return 1; */
	/* } */
	/* else{ */
	/* 	printf("fork failed\n"); */
	/* } */

	
	return 1;
}
