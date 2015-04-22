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
	argc=*(rsp+2);
	argv=(char **)(rsp+3);
	envp=(char **)(rsp+2+argc+2);
	res = main(argc, argv, envp);
	/* res=main(*(rsp+2),(char **)(*(rsp+3)),(char **)(*(rsp+*(rsp+2)+3))); */
	exit(res);
}


