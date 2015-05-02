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
	char *new_argv[2];
	new_argv[0]="bin/sbush";
	new_argv[1]=NULL;
	int b=fork();
	if(b>0){
		while(1){
			yield();
		}
	}
	else if(b==0){
		int ret=execve("bin/sbush",new_argv,envp);
		if(ret==-1)
			printf("exec failed\n");
	}
	else{
		printf("fork failed\n");
	}
	return 0;
}

