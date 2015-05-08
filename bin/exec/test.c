#include<stdio.h>
#include <stdlib.h>
#include <string.h>
void foo();

int main(int argc, char* argv[], char* envp[]) {
	/* infinite recursion test */
	/* foo(); */

	/* segmention fault case 1 */
  	/* char *a=(char*) 0xffffffff80000000; */
	/* a[0]='f'; */
	/* printf("%c\n",a); */

	/* segmentation fault case 2 */
	/* int *a =(int*) 1; */
	/* printf("%d\n", *a); */

	/* infinite execs and forks*/
	/* int status = 0; */

	/* while(1){ */
	/* 	int pid=fork(); */
	/* 	if(pid>0){ */
	/* 		strcat(envp[0], ":added"); */
	/* 		waitpid(-1, &status, 0); */
	/* 	} */
	/* 	else if(pid == 0){ */
	/* 		printf("envp strlen -> %d\n",strlen(envp[0])); */
	/* 		execve("/bin/kmalloc", argv, envp); */
	/* 	} */
	/* 	else{ */
	/* 		printf("fork failed\n"); */
	/* 	} */
	/* } */

	/* printf("%s\n",envp[0]); */
	/* strcat(envp[0],"bin:"); */
	/* int ret=execve("/bin/exec",argv,envp); */
	/* if(ret == -1){ */
	/* 	printf("exec failed, errno->%d\n",errno); */
	/* } */


	/* infinte malloc */
	/* char *a=(char*)malloc(1000000000); */
	/* int i; */
	/* for(i=0;i<1000000000;i+=0x1000){ */
	/* 	a[i]='a'; */
	/* } */
	
	/* ls */

	/* for(int i=0;i<400;i++){ */
	/* 	int pid=fork(); */
	/* 	if(pid>0){ */
	/* 		printf("%d\n",i); */
	/* 		mallocTest(); */
	/* 	} */
	/* 	else if(pid==0){ */
	/* 		/\* return ; *\/ */
	/* 		/\* while(1); *\/ */
	/* 		return 0; */
	/* 	} */
	/* 	else{ */
	/* 		printf("fork failed\n"); */
	/* 		printf("%d\n",i); */
	/* 		mallocTest(); */

	/* 	} */
	/* } */
	return 0;
	/* int status=0; */
	/* printf("test exec successful\n"); */
	/* /\* for(int i=0;i<5;i++) *\/ */
	/* mallocTest(); */
	/* /\* foo(); *\/ */
	/* int pid=fork(); */
	/* if(pid>0){ */
	/* 	printf("1 "); */
	/* 	mallocTest(); */
	/* 	waitpid(pid, &status, 0); */
	/* 	printf("2 "); */
	/* 	mallocTest(); */
	/* } */
	/* else if(pid==0){ */
	/* 	printf("child process\n"); */
	/* 	int pid1=fork(); */
	/* 	if(pid1>0){ */
	/* 		printf("3 "); */
	/* 		mallocTest(); */
	/* 		waitpid(pid1, &status, 0); */
	/* 		printf("4 "); */
	/* 		mallocTest(); */
	/* 		exit(0); */
	/* 	} */
	/* 	else if(pid1==0){ */
	/* 		printf("5 "); */
	/* 		mallocTest(); */
	/* 		sleep(10); */
	/* 		foo(); */
	/* 		exit(0); */
	/* 	} */
	/* } */
	/* while(1) */
	/* 	yield(); */
}
void foo(){
	char a[1000];
	*a='a';
	foo();
}
