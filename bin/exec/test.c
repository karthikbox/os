#include<stdio.h>
#include <stdlib.h>

void foo();

int main(int argc, char* argv[], char* envp[]) {
	/* foo(); */
	for(int i=0;i<1000;i++){
		int pid=fork();
		if(pid>0){
			;
		}
		else if(pid==0){
			return 0;
		}
		else{
			printf("fork failed\n");
		}
	}
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
