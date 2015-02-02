#include<stdio.h>
#include<stdlib.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);
void free_array(char **tokens,int len);
void cmd_cd(char** tokens);
void cmd_set(char** tokens,int token_len,char *envp[],int path_index);
void cmd_binary(char** tokens,int len,char *envp[]);
void cmd_script(char** tokens,int len,char *envp[]);
int isBinary(char** tokens,int len);
int isScript(char** tokens,int len);

void printPrompt(char* str);
char* getpath(int *index, char *envp[]);

int main(int argc, char *argv[],char *envp[]){
  	char name[1000];
	int token_len,index;
	int i;
	//char path
	char prompt[500];
	char *prompt_ret,**tokens,*path;
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
	      path = getpath(&index, envp); //function to get the 'value' of PATH environment variable; gives the index of the same
	      printf("path is %s\n", path);
	      printf("index of path is %d\n", index);
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
		  cmd_set(tokens,token_len,envp,index);
		}
		else
		  printf("incorrect syntax for set command\n");
	      }
	      else if(token_len == 1){
		//binary or script
		if(isBinary(tokens,token_len)){
		  cmd_binary(tokens,token_len,envp);
		}
		else if(isScript(tokens,token_len))
		  cmd_script(tokens,token_len,envp);
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

void cmd_binary(char** tokens,int token_len,char *envp[]){
  int status,id;
  int path_index,tok_len;
  
  int pid=fork();
  if(pid==0){
    //child
    if(execve(tokens[0],tokens,envp)==-1){
      printf("binary file-execve-error  %s\n",tokens[0]);;
      //trying path directories
      char* path_raw=getpath(&path_index,envp);
      char** paths=strtoken(path_raw,":",&tok_len);
      int i;
      for(i=0;i<tok_len;i++){
	if(execve(strcat(paths[i],tokens[0]),tokens,envp)==-1){
	  printf("binary file-execve-error\n");
	}
      }
      //printf("number of paths is %d\n",tok_len);
    }
    printf("%s not found\n",tokens[0]);
    exit(0);
  }
  else if(pid > 0){
    //parent
    while ((id = waitpid(-1,&status,0)) != -1) /* pick up all the dead children */ 
      printf("process %d exits\n", pid); 
  }
  else{
    //error on fork
    printf("error on fork...try again\n");
  }
  
}

void cmd_script(char** tokens,int token_len,char *envp[]){

}

int isScript(char** tokens,int len){
  return 1;
}

int isBinary(char** tokens,int len){
  return 1;
}


void cmd_cd(char** tokens){
  if(chdir(tokens[1])<0)
    printf("error\n");
}

void cmd_set(char** tokens,int token_len,char **envp,int path_index){
	

}

char* getpath(int *index, char **envp){

	int key=0, token_len;
	char **tokens = (char**)malloc(100*sizeof(char*));
	while(envp[key] != '\0')
	{
		tokens = strtoken(envp[key],"=", &token_len);
		if(strcmp(tokens[0], "PATH") == 0)
		{
			*index = key;
			break;
		}
		key++;
	}
	return tokens[1];
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
