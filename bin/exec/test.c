#include<stdio.h>
#include <stdlib.h>

void foo();
int main(int argc, char* argv[], char* envp[]) {
	int status=0;
	printf("test exec successful\n");
	/* for(int i=0;i<5;i++) */
	mallocTest();
	/* foo(); */
	int pid=fork();
	if(pid>0){
		printf("Parent process\n");
		mallocTest();
		waitpid(pid, &status, 0);
		mallocTest();
	}
	else if(pid==0){
		printf("child process\n");
		int pid1=fork();
		if(pid1>0){
			printf("child parent process\n");
			mallocTest();
			waitpid(pid1, &status, 0);
			mallocTest();
			exit(0);
		}
		else if(pid1==0){
			mallocTest();
			printf("child child process\n");
			exit(0);
		}
	}
	while(1)
		yield();
}
void foo(){
	foo();
}
