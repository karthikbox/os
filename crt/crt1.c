#include <stdlib.h>
#include<stdio.h>
uint64_t *get_rsp();

int main(int argc, char* argv[], char* envp[]);

void _start(void) {
	int argc;
	char** argv;
	char** envp;
	int res;
	uint64_t *rsp;
	__asm__
	(
		"mov %%rsp,%0\n\t"
		:"=r" (rsp)
	);
	argc=*(rsp+1);
	argv=(char **)rsp+2;
	envp=(char **)rsp+(argc+3);
	res = main(argc, argv, envp);
	while(1);
	exit(res);
}


