#include <sys/sbunix.h>
#include<sys/syscall.h>
#include<stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	printf("proc 2 exec successful\n");
	sleep(4);
	printf("proc 2 after sleep\n");
	return 0;
}

