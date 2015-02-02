#include<stdio.h>
#include<stdlib.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);
void strcat(char* envPath, char* path);
void free_array(char **tokens,int len);
void cmd_cd(char** tokens);
void cmd_set_path(char* tokens,char *envp[],int path_index);
void cmd_set_ps1(char** tokens);
void cmd_binary(char** tokens,int len,char *envp[]);
void cmd_script(char** tokens,int len,char *envp[]);
int isBinary(char** tokens,int len);
int isScript(char** tokens,int len);

void printPrompt(char* str,int print_prompt_flag);
char* getpath(int *index, char *envp[]);

int main(int argc, char *argv[],char *envp[]){
  	char name[1000];
	int token_len,index;
	int i,PRINT_PROMPT_FLAG=1;
	//char path
	char prompt[500];
	char *prompt_ret,**tokens,*path;
	int in;
	if (argc==2){
	  //execute script
	  printf("%s\n",argv[1]);
	  int open(const char *pathname, int flags);
	  if((in=open(argv[1],O_RDONLY))==-1){
	    printf("unable to read file %s",argv[1]);
	    exit(0);
	  }
	  dup2(in,0);
	  PRINT_PROMPT_FLAG=0;
	}
        while(1)
	    {
	      //printf("sbush@cse506$ ");
            
	      path = getpath(&index, envp); //function to get the 'value' of PATH environment variable; gives the index of the same
	      //printf("path is %s\n", path);
	      //printf("index of path is %d\n", index);
            
          prompt_ret=getcwd(prompt,sizeof(prompt)+1);
          if(prompt_ret!=0)
	    printPrompt(prompt,PRINT_PROMPT_FLAG);
	  if(scanf(" %[^\n]s", name) == -1)
	    break;
            
	      if(strcmp(name, "exit") == 0)
          {
              break;
          }

	      tokens = strtoken(name, " ",&token_len);
          i = 0;
            
	      if(strcmp(tokens[i],"cd")==0)
          {
              if(token_len==2)
                  cmd_cd(tokens);
              else
                  printf("incorrect syntax for cd\n");
	      }
	      else if(strcmp(tokens[i],"set")==0)
          {
              if((strcmp(tokens[i+1], "PATH")==0) && (token_len==3))
              {
                  cmd_set_path(tokens[i+2],envp,index);
                  printf("Path is %s\n", envp[index]);
              }
              else if((strcmp(tokens[i+1], "PS1")==0) && (token_len==2))
                  cmd_set_ps1(tokens);
              else
                  printf("incorrect syntax for set command\n");
	      }
	      else if(token_len >= 1)
          {
              //binary or script
	    if(isScript(tokens,token_len)){
	      if(token_len==2)
		cmd_script(tokens,token_len,envp);
	      else
		printf("input filename\n");
	    }
	    else{
	      //printf("binary\n");
	      cmd_binary(tokens,token_len,envp);
	    }
          }
          else
              printf("unknown command\n");
	      free_array(tokens,token_len);
	    }
	free(path);
  	return 1;

}

void free_array(char **tokens,int len){
  int i;
  for(i=0;i<len;i++)
    free(tokens[i]);
  free(tokens);
}

void printPrompt(char* str,int print_prompt_flag){
  if(print_prompt_flag)
    printf("%s$ ",str);
}

void cmd_binary(char** tokens,int token_len,char *envp[]){
  int status,id;
  int path_index,path_len;
  
  int pid=fork();
  if(pid==0){
    //child
    
    if(execve(tokens[0],tokens,envp)==-1){
      //printf("binary file-execve-error  %s\n",tokens[0]);;
      //trying path directories
      char* path_raw=getpath(&path_index,envp);
      char** paths=strtoken(path_raw,":",&path_len);
      char *org=tokens[0];
      int i;
      for(i=0;i<path_len;i++){
	strcat(paths[i],"/");
	strcat(paths[i],org);
	tokens[0]=paths[i];
	if(execve(paths[i],tokens,envp)==-1){
	  //printf("binary file-execve-error %s\n",paths[i]);
	  ;
	}
      }
      printf("cannot execute %s\n",org);
      //free_array(tokens,token_len); //getting error double free 
      //maybe because of copy on write which linux follows
      free_array(paths,path_len);
      free(path_raw);
    }
    exit(0);
  }
  else if(pid > 0){
    //parent
    while ((id = waitpid(-1,&status,0)) != -1) /* pick up all the dead children */ 
      //printf("process %d exits\n", pid); 
      ;
  }
  else{
    //error on fork
    printf("error on fork...try again\n");
  }
  
}

void cmd_script(char** tokens,int token_len,char *envp[]){
  int status,id;
  int pid=fork();
  if(pid==0){
    //child
    char *params[]={"./rootfs/bin/sbush",0,0};
    params[1]=tokens[1];
    if(execve(params[0],params,envp)==-1){
      printf("unable to create sbush child\n");
    }
    exit(0);
  }
  else if(pid > 0){
    //parent
    while ((id = waitpid(-1,&status,0)) != -1) /* pick up all the dead children */ 
      //printf("process %d exits\n", pid); 
      ;
  }
  else{
    //error on fork
    printf("error on fork...try again\n");
  }
  
}

int isScript(char** tokens,int len){
  if(strcmp(tokens[0],"sbush")==0)
    return 1;
  else 
    return 0;
}

int isBinary(char** tokens,int len){
  return 1;
}


void cmd_cd(char** tokens){
  if(chdir(tokens[1])<0)
    printf("error\n");
}

void cmd_set_path(char* path, char **envp, int path_index){
	strcat(envp[path_index], path);
}

void cmd_set_ps1(char** tokens)
{
    
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
	free(tokens[0]);
	char *res=tokens[1];
	free(tokens);
	return res;
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
  int TOKEN_ARRAY_SIZE=100;
  int TOKEN_SIZE=300;
  char **tokens = (char**)malloc(TOKEN_ARRAY_SIZE*sizeof(char*));

  tokens[j] = (char*)malloc(TOKEN_SIZE*sizeof(char));



  while(s[i] != '\0'){
    if(s[i] == *delim)
      {
	tokens[j][k] = '\0';
	j++;
	tokens[j] = (char*)malloc(TOKEN_SIZE*sizeof(char));
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
