#include<stdio.h>
#include<stdlib.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim);

int main(int argc, char *argv[],char *envp[]){
  	char name[1000];
  
  	while(1)
  	{

	  	printf("sbush@cse506$ ");
	  	scanf(" %[^\n]s", name);
	  	if(strcmp(name, "exit") == 0)
	  	{
	  		break;
	  	}

	  	char** tokens = strtoken(name, " ");
	  	int i = 0;

	  	while(tokens[i] != 0){
	  		printf("%s\n", tokens[i]);
	  		i++;
  		}
	}
  	return 1;

}

int strcmp(const char *s1,const char * s2){
	int i=0;
	for(i=0;s1[i]==s2[i];i++){
	     if(s1[i]=='\0')
	 	     return 0;
	}
	return s1[i]-s2[i];
}

char** strtoken(const char *s, const char *delim){
int i=0, j=0, k=0;
char **tokens = (char**)malloc(100*sizeof(char*));

tokens[j] = (char*)malloc(200*sizeof(char));



while(s[i] != '\0'){
	if(s[i] == *delim)
	{
		tokens[j][k] = '\0';
		j++;
		tokens[j] = (char*)malloc(200*sizeof(char));
		k=0;
	}
	else
	{
		tokens[j][k] = s[i];
		k++;			
	}
	i++;
}
tokens[j][k] = '\0';
tokens[++j] = 0;


return tokens;
}
