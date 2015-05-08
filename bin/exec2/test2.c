#include<stdio.h>
#include <stdlib.h>
#include<string.h>

void foo(){
	foo();
}
int main(int argc, char* argv[], char* envp[]) {
	int a;
	scanf("%d",&a);
	a++;
	printf("%d\n",a);

	
	/* int ret=dup(STDOUT); */
	/* if(ret == -1){ */
	/* 	printf("ret->"); */
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
	


	/* int64_t *ret=(int64_t *)sbrk(0x1000); */

	/* if(ret < 0 ){ */
	/* 	printf("fail, errno->%d\n",errno); */
	/* } */
	/* else{ */
	/* 	printf("pass, ret->%x\n",(uint64_t)ret); */
	/* 	ret=(int64_t *)sbrk(0x1000); */
	/* 	printf("pass, ret->%x\n",(uint64_t)ret); */
	/* } */


	/* int i=0; */
	/* for(i=0;i<2400;i++){		 */
	/* 	int a=fork(); */
	/* 	if(a > 0){ */
	/* 		printf("rn %d\n",i); */
	/* 	} */
	/* 	else if(a==0){ */
	/* 			sleep(30); */
	/* 			exit(1); */

	/* 	} */
	/* 	else{ */
	/* 		printf("fork failed at %d run, errno -> %d",i,errno); */
	/* 	} */
	/* } */
	


	/* int a=fork(); */
	/* if(a > 0){ */
	/* 	printf("parent\n"); */
	/* 	int status; */
		
	/* 	int b=fork(); */

	/* 	if(b > 0){ */
	/* 		int ret=waitpid(-1,&status,0); */
	/* 		printf("after parent. ret->%d, errno->%d\n",ret,errno); */
			
	/* 	} */
	/* 	else if( b==0){ */
	/* 		sleep(9); */
	/* 		printf("child 2\n"); */
	/* 		exit(0); */
	/* 	} */
	/* 	else{ */
	/* 		printf("sec child failed\n"); */
	/* 	}		 */
	/* } */
	/* else if(a==0){ */
	/* 	printf("child 1\n"); */
	/* 	sleep(3); */
	/* 	exit(0); */
	/* } */
	/* else{ */
	/* 	printf("fork failed\n"); */
	/* } */

}
