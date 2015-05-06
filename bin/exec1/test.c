#include<stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	/* printf("test exec1 successful\n"); */
	/* scanf("%d", &n); */
	/* n++; */
	/* printf("%d\n",n); */
	/* char* a=(char *) malloc(1000); */
	/* for(int i=0;i<1000000000;i=i+0x1000) */
	/* 	a[i]='a'; */
	int *a=(int*)0xffffffff80000000;
	*a=1;
	printf("%d\n",*a);
		
}
