#include<stdio.h>
#include<stdlib.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);
void cmd_cd(char** tokens);
void cmd_set(char** tokens);
void printPrompt(char* str);

int main(int argc, char *argv[],char *envp[]){
  	char name[1000];
	int token_len;
	int i;
	//char path
	char prompt[500];
	char *prompt_ret;
  	while(1)
  	{
	  //printf("sbush@cse506$ ");
	  prompt_ret =getcwd(prompt,sizeof(prompt)+1);
	  if(prompt_ret!=0)
		printPrompt(prompt);
	  	scanf(" %[^\n]s", name);
	  	if(strcmp(name, "exit") == 0)
	  	{
	  		break;
	  	}

	  	char** tokens = strtoken(name, " ",&token_len);
	  	i = 0;
		if(strcmp(tokens[i],"cd")==0){
		  if(token_len==2)
		    cmd_cd(tokens);
		  else
		    printf("incorrect syntax for cd\n");
		}
		else if(strcmp(tokens[i],"set")==0){
		  if(token_len==2||token_len==3){
		    cmd_set(tokens);
		  }
		  else
		    printf("incorrect syntax for cd\n");
		}
	}
  	return 1;

}

void printPrompt(char* str){
  printf("%s$ ",str);
}

void cmd_cd(char** tokens){
  if(chdir(tokens[1])<0)
    printf("error\n");
}

void cmd_set(char** tokens){

}

int strcmp(const char *s1,const char * s2){
	int i=0;
	for(i=0;s1[i]==s2[i];i++){
	     if(s1[i]=='\0')
	 	     return 0;
	}
	return s1[i]-s2[i];
}

char** strtoken(const char *s, const char *delim,int *len){
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
  *len=j;

  return tokens;
}
