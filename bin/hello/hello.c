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
	pid_t b=fork();
	if(b>0){
		printf("parent says hi\n");
		printf("parent waiting for child process %d\n",b);
		printf("argc is %d\n",argc);
		/* pid=waitpid(-1,&status,0); */
		char *a=(char *)malloc(10000*sizeof(char));
		printf("a->%x\n",(uint64_t)a);
		int x;
		for(x=0;x<10;x++){
			read(STDIN,a,1);
			printf("parent a->%s\n",a);
		}

		read(STDIN,a,5);
		printf("parent a->%s\n",a);
		read(STDIN,a,3);
		printf("parent a->%s\n",a);
		yield();
	}
	else if(b==0){
		printf("child says hi\n");
	}
	printf("parent child exited %d\n",b);
	while(1)
		yield();
	return 0;
}

