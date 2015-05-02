#include<stdlib.h>
#include<stdio.h>
#include<string.h>


int main(int argc, char* argv[], char* envp[]) {
	printf("hello\n");
	int i;
	for(i=0;argv[i];i++){
		printf("argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("envp[%d]->%s\n",i,envp[i]);
	}
	char *new_argv[2];
	new_argv[0]="bin/sbush";
	new_argv[1]=NULL;
	int b=fork();
	if(b>0){
		while(1){
			yield();
		}
	}
	else if(b==0){
		int ret=execve("bin/sbush",new_argv,envp);
		if(ret==-1)
			printf("exec failed\n");
	}
	else{
		printf("fork failed\n");
	}
	return 0;
}

