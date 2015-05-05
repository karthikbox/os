#include<stdio.h>
#include <stdlib.h>

void foo(){
	foo();
}

int main(int argc, char* argv[], char* envp[]) {
	printf("test exec successful\n");
	perror("test after sleep\n");
	foo();
	return 0;
}

