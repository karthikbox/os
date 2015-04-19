#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	

	int x=1;

	/* printf("Enter a character "); */
	
	/* printf("Entered character is %c\n", ch); */
	uint64_t *ptr=(uint64_t *)(0xfffffeff70000000ul);
	*(uint64_t *)((char *)ptr-0x1001)=234;
	*(uint64_t *)((char *)ptr-0x2001)=234;
	/* fork(); */
	/* if(frk > 0){ */
	/* 	printf("parent\n"); */
	/* } */
	/* else if(frk==0){ */
	/* 	printf("child\n"); */
	/* } */
	/* else{ */
	/* 	printf("fork fail\n"); */
	/* } */
	/* yield(); */
	yield();
	pid_t ret=fork();
	if(ret>0){
		yield();
		*(uint64_t *)((char *)ptr-0x1001)=567;
		*(uint64_t *)((char *)ptr-0x2001)=567;
		*(uint64_t *)((char *)ptr-0x1001)=789;
		*(uint64_t *)((char *)ptr-0x2001)=789;
		yield();
	}
	else if(ret==0){
		while(1)
			x++;
	}
	else{
		while(1)
			x++;
	}
	/* child */
	*(uint64_t *)((char *)ptr-0x1001)=111;
	*(uint64_t *)((char *)ptr-0x2001)=111;
	yield();
	
	return 0;
}

