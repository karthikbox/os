#include<stdio.h>
#include <stdlib.h>

void foo(){
	foo();
}

int main(int argc, char* argv[], char* envp[]) {
	printf("5\n");
	perror("test after sleep\n");
	/* foo(); */
	return 0;
}

