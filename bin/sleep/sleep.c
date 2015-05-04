#include<stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	int nsecs;
	/* printf("Sleep binary called\n"); */
	if(argc==1){
		return 0;
	}

	atoi(argv[1], &nsecs, 10);
	sleep(nsecs);
	return 0;
}

