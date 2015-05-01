#include<stdlib.h>
#include<stdio.h>

int strcmp(const char *s1,const char * s2) {
    int i=0;
    for(i=0; s1[i]==s2[i]; i++) {
        if(s1[i]=='\0')
            return 0;
    }
    return s1[i]-s2[i];
}



int strlen(char *s)
{
    int i = 0;
    while(s[i] != '\0')
        i++;
    return i;

}

void strcat(char* s1, char* s2)
{
    int i=0, j=0;
    while (s1[i] != '\0')
    {
        i++;
    }
    while (s2[j] != '\0')
    {
        s1[i] = s2[j];
        i++;
        j++;
    }
    s1[i] = '\0';
}

void strcpy(char* dest, char* src)
{
    int i=0;
    while(src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}


int main(int argc, char* argv[], char* envp[]) {
	printf("hello\n");
	int i;
	for(i=0;argv[i];i++){
		printf("argv[%d]->%s\n",i,argv[i]);
	}
	for(i=0;envp[i];i++){
		printf("envp[%d]->%s\n",i,envp[i]);
	}
	/* int /\* status=0, *\/pid=0; */
	int fd[2];
	if(pipe(fd)==-1){
		printf("pipe failed\n");
	}
	pid_t b;
	/* int run=0; */
	/* for(;run<65;run++){ */
	/* 	b=fork(); */
	/* 	if(b>0){ */
	/* 		printf("run = %d && pid = %d\n",run,b); */
	/* 		continue; */
	/* 	} */
	/* 	else if(b==0){ */
	/* 		printf("run = %d && pid = %d\n",run,b); */
	/* 		exit(0); */
	/* 	} */
	/* 	else{ */
	/* 		printf("run = %d && pid = %d\n",run,b); */

	/* 	} */
	/* } */
	/* yield(); */
	b=fork();
	char *x="pipe content###\n";
	if(b>0){
		printf("parent says hi\n");
		close(fd[0]);			/* parent closing read end */
		/* dup2 stdout to write end of pipe */
		int ret=dup2(fd[1],STDOUT);
		if(ret==-1)
			printf("dup2 failed\n");
		printf("%s",x);
		yield();
		printf("hello my son\n");
		yield();
	}
	else if(b==0){
		/* int k=fork(); */
		int k=3;
		if(k>0){
			printf("child says hi\n");
			close(fd[1]);		/* child closing write end */
			/* dup2 STDIN to read end of pipe  */
			dup2(fd[0],STDIN);
			char t[100];
			scanf("%s",t);
			printf("child  read -> %s\n",t);
			scanf("%s",t);
			printf("child  read -> %s\n",t);
			scanf("%s",t);
			printf("child  read -> %s\n",t);
			exit(0);
		}
		else if(k==0){
			printf("child child says hi\n");
			while(1)
				yield();
		}
	}
	else{
		printf("fork failed\n");
	}
	while(1)
		yield();
	return 0;
}

