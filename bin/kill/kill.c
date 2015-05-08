#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[], char* envp[]){
	int pid=0;

	if(argc<3){
		printf("kill: Too few arguments\n");
		return 0;
	}

	if(argc>3){
		printf("kill: Too many arguments\n");
	}

	if(strcmp(argv[1],"-9")!=0){
		printf("Usage: kill -9 <pid>\n");
		return 0;
	}

	atoi(argv[2], &pid, 10);
	if(kill(pid)==-1){
		printf("%s: No such process\n", argv[2]);
	}
	return 0;
}
