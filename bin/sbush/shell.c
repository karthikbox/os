#include<stdio.h>
#include<stdlib.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);
void free_array(char **tokens,int len);
void cmd_cd(char** tokens);
void cmd_set(char** tokens);
void cmd_binary(char** tokens,int len,char *envp[]);
void cmd_script(char** tokens,int len,char *envp[]);

void printPrompt(char* str);

int main(int argc, char *argv[],char *envp[]){
  	char name[1000];
	int token_len;
	int i,env_len=0;
	//char path
	char prompt[500];
	char *prompt_ret,**tokens,**env;
	if (argc==2){
	  //execute script
	  
	}
	else{
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

	      tokens = strtoken(name, " ",&token_len);
	      i = 0;
	      if(strcmp(tokens[i],"cd")==0){
		if(token_len==2)
		  cmd_cd(tokens);
		else
		  printf("incorrect syntax for cd\n");
	      }
	      else if(strcmp(tokens[i],"set")==0){
		if(token_len==2||token_len==3){
		  cmd_set(tokens,env,&env_len);
		}
		else
		  printf("incorrect syntax for set command\n");
	      }
	      else if(token_len == 1){
		//binary or script
		if(isBinary(tokens,token_len)){
		  cmd_binary();
		}
		else if(isScript(tokens,token_len))
		  cmd_script();
	      }
	      else
		printf("unknown command\n");
	      free_array(tokens,token_len);
	    }
	}
  	return 1;

}

void free_array(char **tokens,int len){
  int i;
  for(i=0;i<len;i++)
    free(tokens[i]);
  free(tokens);
}

void printPrompt(char* str){
  printf("%s$ ",str);
}

void cmd_binary(char** tokens,int len,char *envp[]){
  
  int pid=fork();
  if(pid==0){
    //child
    if(execve(tokens[0],tokens,envp)==-1){
      printf("binary file-execve-error\n");
    }
  }
  else if(pid > 0){
    //parent
    
  }
  else{
    //error on fork
    printf("error on fork...try again\n");
  }
  
}

void cmd_script(char** tokens,int len){

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
