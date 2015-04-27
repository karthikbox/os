#include<stdlib.h>
#include<stdio.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("hello\n");
	int i;
	for(i=0;argv[i];i++){
		printf("argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("envp[%d]->%s\n",i,envp[i]);
	}
	int status=0,pid=0;
	pid_t b=fork();
	if(b>0){
		printf("parent says hi\n");
		printf("parent waiting for child process %d\n",b);
		printf("argc is %d\n",argc);
		pid=waitpid(-1,&status,0);
		char *a=(char *)malloc(0x10000*sizeof(char));
		printf("enter string\n");
		scanf("%s",a);
		printf("a->%s\n",a);
		/* int num; */
		/* printf("enter number\n"); */
		/* scanf("%d",&num); */
		/* printf("num->%d\n",num); */
		/* printf("enter string\n"); */
		/* scanf("%s",a+0x1000); */
		/* printf("a->%s\n",a+0x1000); */
		/* printf("enter string\n"); */
		/* scanf("%s",a+0x2000); */
		/* printf("a->%s\n",a+0x2000); */
		/* printf("enter number\n"); */
		/* scanf("%d",&num); */
		/* printf("num->%d\n",num); */

		yield();
	}
	else if(b==0){
		int k=fork();
		if(k>0){
			printf("child says hi\n");
			sleep(4);			
			printf("child says hi again\n");
			execve("bin/exec",argv,envp);
			exit(0);
		}
		else if(k==0){
			printf("child child says hi\n");
			while(1)
				yield();
		}
	}
	printf("parent child exited %d && %d\n",b,pid);
	while(1)
		yield();
	return 0;
}

