#include <stdlib.h>
#include<stdio.h>
uint64_t *get_rsp();

void _start(void) {
	int argc,i;
	char** argv;
	char** envp;
	int res;
	uint64_t *rsp;
	__asm__
	(
		"mov %%rsp,%0\n\t"
		:"=r" (rsp)
	);
	rsp=rsp+7;	//rsp+1 implies after 8 bits we have 
//argc
	argc=*rsp;
	argv=(char **)rsp+1;
	printf("argc val %d \n",argc);
	i=0;
	while(i<argc){
		printf("rsp val %s \n",argv[i]);
		i++;
	}
//	argv=*(rsp+1);

	//printf("%s\n",argv);
	//envp=rsp;
	envp=(char **)rsp+(argc+2);
	i=0;
	printf("30 is %s \n",envp[30]);
	printf("31 is %s \n",envp[31]);
	while(envp[i]){
	  printf("%d\n",i);
	  if(i==30){
	    i++;
	    continue;
	  }
	  printf("rsp val %s \n",envp[i]);
	  i++;
	}
	res = main(argc, argv, envp);
	exit(res);
}


