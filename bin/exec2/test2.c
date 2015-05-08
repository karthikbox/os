#include<stdio.h>
#include <stdlib.h>
#include<string.h>

void foo(){
	foo();
}
int main(int argc, char* argv[], char* envp[]) {
	
	/* int fd=open("mnt/home/a.txt",O_RDONLY); */
	/* if(fd < 0){ */
	/* 	printf("open error. errno val->%d\n",errno); */
	/* 	exit(0); */
	/* } */
	/* char *a=(char *)malloc(1000); */
	/* int ret=read(fd,a,1000); */
	/* if(ret <0){ */
	/* 	printf("read failed. errno->%d\n",errno); */
	/* 	exit(0); */
	/* } */
	/* printf("ret->%d\n",ret); */
	/* printf("%s\n",a); */
	
	/* return 1; */

	int a=fork();
	if(a > 0){
		printf("parent\n");
		int status;
		
		int b=fork();

		if(b > 0){
			int ret=waitpid(-1,&status,0);
			printf("after parent. ret->%d, errno->%d\n",ret,errno);
			
		}
		else if( b==0){
			sleep(9);
			printf("child 2\n");
			exit(0);
		}
		else{
			printf("sec child failed\n");
		}		
	}
	else if(a==0){
		printf("child 1\n");
		sleep(3);
		exit(0);
	}
	else{
		printf("fork failed\n");
	}

}
