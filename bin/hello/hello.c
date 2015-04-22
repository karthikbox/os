#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	uint64_t *ptr=(uint64_t *)0xfffffeff70000000ul;
	*(uint64_t *)((char *)ptr-0x1001)=234;
	*(uint64_t *)((char *)ptr-0x2001)=234;
	yield();
	/*printf("break pointer is %x\n", (uint64_t)sbrk(0));*/
	int *p = (int*)malloc(sizeof(int));
	*p=10;
	printf("p is %d\n", *p);
	int *q = (int*)malloc(sizeof(int));
	*q=100;
	printf("q is %d %d\n", getpid(), getppid());
	free(p);
	free(q);
	
	pid_t ret=fork();
	printf("fork return -> %d\n",ret);
	if(ret>0){
		printf("parent says hi parent %d, child %d\n",getppid(),getpid());
		yield();
		printf("1 return -> %d\n",ret);
		yield();
	}
	else if(ret==0){
	  printf("child says hi parent %d, child %d\n",getppid(),getpid());
		pid_t ret1=fork();
		printf("ret1 is %d\n", ret1);
		if(ret1>0){
		  printf("child_parent parent %d, child %d\n",getppid(),getpid());
		  yield();
		  printf("child_parent after yield\n");
		}
		else if(ret1==0){
		  printf("child_child parent %d, child %d\n",getppid(),getpid());
		  exit(0);
		}
		exit(0);
	}
	else{
		printf("fork failed\n");
		}
	/* child */
	*(uint64_t *)((char *)ptr-0x1001)=111;
	printf("2 return -> %d\n",ret);
	while(1);
	return 0;
}

