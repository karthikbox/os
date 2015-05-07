#include<stdio.h>
#include <stdlib.h>
#include<string.h>
int main(int argc, char* argv[], char* envp[]) {
	printf("exec2");
	char a[100];
	scanf("%s",a);
	printf("exec2 says %s\n",a);
	return 1;
}
