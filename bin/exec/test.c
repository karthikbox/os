#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	int i;
	for(i=0;argv[i];i++){
		printf("test argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("test envp[%d]->%s\n",i,envp[i]);
	}
	sleep(4);
	printf("proc 4 after sleep\n");
	return 0;
}

