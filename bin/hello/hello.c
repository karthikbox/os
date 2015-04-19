#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	
	uint64_t *ptr=(uint64_t *)(0xfffffeff70000000ul);
	*(uint64_t *)((char *)ptr-0x1001)=234;
	*(uint64_t *)((char *)ptr-0x2001)=234;
	yield();
	pid_t ret=fork();
	if(ret>0){
		printf("parent says hi\n");
		yield();
		*(uint64_t *)((char *)ptr-0x1001)=567;
		*(uint64_t *)((char *)ptr-0x1001)=789;
		yield();
	}
	else if(ret==0){
		printf("child says hi\n");
	}
	else{
		printf("fork failed\n");
	}
	/* child */
	*(uint64_t *)((char *)ptr-0x1001)=111;
	yield();
	
	return 0;
}

