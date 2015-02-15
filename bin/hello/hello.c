#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[], char* envp[]) {
	
	//test printf
    
	//char course[] = "Operating Systems";
	//char semester[] = "%Spring";
	//int year = 2015;
	//int hex = 255;
	/*char c = 'a';
	printf("Course\t\t\tSemester\tYear\n");
	printf("%s\t%s\t\t%d\n", course, semester, year);
	printf("Decimal - %d; Hexadecimal - %x\n", hex, hex);
	printf("The character is %c\n", c);*/
	/*printf("\n%d", year);
	printf("\nbetween");
	printf("\n%d", hex);*/

	//test scanf
	char buf[100];
	printf("Enter a string\n");
	scanf("%s", buf);
	printf("Entered string is %s\n", buf);

	//test execve
/*
	printf("argc is %d\n", argc);
	char *tokens[] = {"/bin/ls", "-alh", 0};
	if(execve(tokens[0], tokens, envp)==-1) {
		printf("execve errorn\n");
	}
*/
    //TEST FOR opendir,readdir,closedir
    /*void *dirp=opendir(argv[1]);
    if(!dirp){
	printf("opendir failed\n");
	exit(1);
    }
    struct dirent *ptr;
    while((ptr=readdir(dirp))){
	printf("%s\n",ptr->d_name);
    }
    printf("\n\n");
    void *dirp2=opendir(argv[2]);
    if(!dirp2){
	printf("opendir failed\n");
	exit(1);
    }
    struct dirent *ptr2;
    while((ptr2=readdir(dirp2))){
	printf("%s\n",ptr2->d_name);
    }

    if(closedir(dirp)==-1){
	printf("closedir failed\n");
	exit(1);
    }
    if(closedir(dirp2)==-1){
	printf("closedir failed\n");
	exit(1);
    }*/

    //TEST argv, argv, envp
    /*
    int i=0;
    printf("argc is %d \n",argc);
    while(argv[i]){
	printf("argv is %s \n",argv[i]);
	i++;
    }
    i=0;
    while(envp[i]){
	printf("envp is %s \n",envp[i]);
	i++;
    }
    */
    //TEST PRINTF
    /*
	char course[] = "Operating Systems";
	char semester[] = "%Spring";
	int year = 2015;
	int hex = 255;
	char c = 'a';
	printf("Course\t\t\tSemester\tYear\n");
	printf("%s\t%s\t\t%d\n", course, semester, year);
	printf("Decimal - %d; Hexadecimal - %x\n", hex, hex);
	printf("The character is %c\n", c);
    */

    /*
      uint64_t a=(uint64_t)sbrk(100);
      uint64_t b=(uint64_t)sbrk(0);
      printf("%x\n",a);
      printf("%x\n",b);
    */

    // MALLOC AND FREE TEST CASE 
    //char *temp=(char *)malloc(100);//blk 1 
    /* if(temp==NULL) */
    /* 	printf("error in malloc\n"); */
    //char *b=(char *)malloc(10);//blk 2 
    /* int i=0; */
/*  printf("%d\n",temp);
    printf("%d\n",b);
    free(temp);//free blk 1
    char *c=(char *)malloc(10);//blk 1 is split as blk1.1 and blk1.2
    printf("%d\n",c);//c gets blk1.1 and blk1.2 is marked free
    char *d=(char *)malloc(10);//blk1.2 is d
    printf("%d\n",d);
    char *e=(char *)malloc(10);//blk 3 is e
    printf("%d\n",e);
    free(e);//free blk 3 and brk reduces as e is the last blk
    e=(char *)malloc(10);//e gets again blk 3
    printf("%d\n",e);//same address as blk 3
    free(e);
    e=(char *)malloc(10);//same as above
    printf("%d\n",e);//same as above
    free(e);//same
    e=(char *)malloc(10);//same
    printf("%d\n",e);//same
    free(d);//free blk1.2
    d=(char *)malloc(10);//d gets blk1.2 again
    printf("%d\n",d);//same as previous address
*/

	//test fork
    int pid, ppid,status; //id, status;
	ppid = getpid();
	pid = fork();
	if(pid == 0) {
		if(ppid == getppid())
			printf("getppid() working\n");
		sleep(5);
		printf("This is child process\n");
		return 0;
	}
	else if(pid>0) {
		waitpid(pid,&status,0);
		printf("This is parent process\n");
	}
	else
		printf("Fork error\n");
		

	//test getcwd
	/*char buf[500];
	getcwd(buf,sizeof(buf)+1);
	printf(buf);
	printf("\n");*/

	//test open, read, write, dup, dup2
	/*int n, newfd = 6, dupfd = 7;
	char buf[500];
	n=open("/home/stufs1/smaguluri/warmup1/s15-w1/LICENSE",O_RDONLY);
	if(n > 0)
	{
		printf("File opened successfully\n");
		read(n, buf, sizeof(buf));
		write(1, buf, sizeof(buf));
	}
	printf("\n");

	dup2(n, newfd);
	printf("After dupping\n");
	read(newfd, buf, sizeof(buf));
	write(1, buf, sizeof(buf));
	printf("\n");

	dupfd = dup(1);
	write(dupfd, "dup working\n", 12);
	printf("\n");

	close(newfd);
	if(read(newfd, buf, sizeof(buf) < 0))
		printf("Read error\n");*/

	//test pipe
	/*int n;
	int fd[2];
	char buf[1025];
	char *data = "hello";

	pipe(fd);
	write(fd[1], data, 5);
	if ((n = read(fd[0], buf, 1024)) >= 0) {
		buf[n] = 0;	
		printf("read bytes from the pipe : ");
		printf(buf);
		printf("\n");
	}	
	else
		printf("read error\n");
	exit(0); */

	//test lseek
	/*int n;
	char buf[500];
	n=open("/home/stufs1/smaguluri/warmup1/s15-w1/LICENSE",O_RDONLY);
	read(n, buf, sizeof(buf));
	write(1, buf, sizeof(buf));
	printf("\nAfter lseek\n");
	lseek(n,1,SEEK_CUR);
	read(n, buf, sizeof(buf));
	write(1, buf, sizeof(buf));
	printf("\n");*/

	return 0;
}
