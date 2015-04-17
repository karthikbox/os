#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	
	char ch='X';
	/* __asm__ __volatile__("movq $24,%%rax;" */
	/* 					 "int $0x80;"  */
	/* 					 :  */
	/* 					 : */
	/* 					 : */
	/* 					 ); */
	

	/* printf("Enter a character "); */
	
	/* printf("Entered character is %c\n", ch); */
	uint64_t *ptr=(uint64_t *)(0xffffffff70000000ul-0x3000ul);
	*ptr=1234;
	yield();
	while(1){
		ch++;
		/* printf("Entered character is %c\n", ch); */
	}
	return 0;
}

