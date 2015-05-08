#include<stdio.h>
#include <stdlib.h>
#include<string.h>

void foo(){
	foo();
}
int main(int argc, char* argv[], char* envp[]) {
	int d;
	scanf("%d",&d);
	d++;
	printf("%d\n",d);
	
	/* printf("exec2"); */
	/* char a[100]; */
	/* scanf("%s",a); */
	/* printf("exec2 says %s\n",a); */
	return 1;
}
