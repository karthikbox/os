#include<stdio.h>
#include<stdlib.h>

#define TOKEN_SIZE 2000
#define TOKEN_ARRAY_SIZE 200
int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);
void strcat(char* envPath, char* path);
void strcpy(char* dest, char* src);
int strlen(char *s);
void free_array(char **tokens,int len);
void cmd_cd(char** tokens);
void cmd_set_path(char* tokens,char *envp[],int path_index,char* envpath);
int cmd_set_ps1(char* command, char** ps1);
void cmd_binary(char** tokens,int len,char *envp[]);
void cmd_script(char** tokens,int len,char *envp[]);
int cmd_pipe_init(char **tokens, int pipes, char **envp);
int isBinary(char** tokens,int len);
int isScript(char** tokens,int len);
int runPipe(char *tokens[],char *envp[], int pipes);
void trim(char *s);
void printPrompt(char* str,int print_prompt_flag);
char* getpath(int *index, char *envp[]);

int main(int argc, char *argv[],char *envp[]) {
    char name[1000];
    int token_len,index;
    int i,PRINT_PROMPT_FLAG=1, ps1Flag=0, isValid;
    //char path
    char prompt[500];
    char *prompt_ret,**tokens,*path;
    int in;
    if (argc==2) {
        //execute script
        int open(const char *pathname, int flags);
        if((in=open(argv[1],O_RDONLY))==-1) {
            printf("unable to read file %s\n",argv[1]);
            exit(0);
        }
        dup2(in,0);
        PRINT_PROMPT_FLAG=0;
    }
    while(1)
    {
        //printf("sbush@cse506$ ");

        path = getpath(&index, envp); //function to get the 'value' of PATH environment variable; gives the index of the same
        if(ps1Flag == 0)
        {
            prompt_ret=getcwd(prompt,sizeof(prompt)+1);
            if(prompt_ret!=NULL)
            {
                strcat(prompt,"$ ");
                printPrompt(prompt,PRINT_PROMPT_FLAG);
            }
	    else{
		printf("getcwd failed.\n");
		exit(1);
	    }
        }
        else
              printPrompt(prompt,PRINT_PROMPT_FLAG);

        
        if(scanf("%s", name) == -1)
            break;
        

        if(strcmp(name, "exit") == 0)
        {
            break;
        }
        
        trim(name);
        tokens = strtoken(name, "|", &token_len);
        if(token_len > 1) //pipes
        {
            cmd_pipe_init(tokens, token_len-1, envp);
        }
        
        else //excluding pipes
        {      
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
                if((strcmp(tokens[i+1], "PATH")==0) && (token_len == 3))
                {
                      cmd_set_path(tokens[i+2],envp,index,path);
                      printf("%s\n", envp[index]);
                }
                else if((strcmp(tokens[i+1], "PS1")==0) && (token_len >=3))
                {
                    isValid = cmd_set_ps1(name, tokens);
                    if(isValid)
                    {
                        strcpy(prompt, tokens[0]);
                        ps1Flag = 1;
                    }
                    else
                        printf("Incorrect syntax for PS1 command\n");
                }
                else
                      printf("incorrect syntax for set command\n");
            }
            else if(token_len >= 1)
            {
                //binary or script
                if(isScript(tokens,token_len)) {
                    if(token_len==2)
                        cmd_script(tokens,token_len,envp);
                    else
                        printf("input filename\n");
                }
                else {
                    //printf("binary\n");
                    cmd_binary(tokens,token_len,envp);
                }
            }
            else
                printf("unknown command\n");
            free_array(tokens,token_len);
        }
    }
    free(path);
    return 1;

}

void free_array(char **tokens,int len) {
    int i;
    for(i=0; i<len; i++)
        free(tokens[i]);
    free(tokens);
}

void printPrompt(char* str,int print_prompt_flag) {
    if(print_prompt_flag)
        printf("%s",str);
}

void cmd_binary(char** tokens,int token_len,char *envp[]) {
    int status;
    int path_index,path_len;
    int pid=fork();
    if(pid==0) {
        //child
      	
      
	//printf("child\n");
        if(execve(tokens[0],tokens,envp)==-1) {
            //printf("binary file-execve-error  %s\n",tokens[0]);;
            //trying path directories
            char* path_raw=getpath(&path_index,envp);
            trim(path_raw);
            char** paths=strtoken(path_raw,":",&path_len);
            char *org=tokens[0];
            int i;
            for(i=0; i<path_len; i++) {
                strcat(paths[i],"/");
                strcat(paths[i],org);
                tokens[0]=paths[i];
                if(execve(paths[i],tokens,envp)==-1) {
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
    else if(pid > 0) {
        if(pid == waitpid(pid,&status,0)){ /* pick up all the dead children */ 
	    //printf("child exit successful\n");
	    ;
	}
	else{
	    printf("error in waitpid. Error no is %d\n",status);
	    exit(1);
	}
    }
    else {
        //error on fork
        printf("error on fork...try again\n");
    }

}

void cmd_script(char** tokens,int token_len,char *envp[]) {
    int status;
    int pid=fork();
    if(pid==0) {
        //child
        char *params[]= {"./rootfs/bin/sbush",0,0}; //hardode must fix this
        params[1]=tokens[1];
        if(execve(params[0],params,envp)==-1) {
            printf("unable to create sbush child\n");
        }
        exit(0);
    }
    else if(pid > 0) {
        //parent
        if(pid == waitpid(pid,&status,0)){ /* wait till child exits */ 
	    //printf("child exit successful\n");
	    ;
	}
	else{
	    printf("error in waitpid. Error no is %d\n",status);
	    exit(1);
	}
    }
    else {
        //error on fork
        printf("error on fork...try again\n");
    }

}

int isScript(char** tokens,int len) {
    if(strcmp(tokens[0],"sbush")==0)
        return 1;
    else
        return 0;
}

int isBinary(char** tokens,int len) {
    return 1;
}


void cmd_cd(char** tokens) {
    if(chdir(tokens[1])<0)
        printf("error\n");
}

void cmd_set_path(char* cmdpath, char **envp, int path_index, char* envpath)
{
    int token_len=0, i=0;
    char *sbushPath = (char*)malloc(TOKEN_SIZE*sizeof(char));
    char **tokens;

    trim(cmdpath);//removes leading and trailing white spaces
    tokens = strtoken(cmdpath, ":", &token_len);

    //printf("path length is %d, %s\n", token_len, tokens[token_len-1]);
    
    for (i=0; i<token_len; i++)
    {
        if(i>0)
            strcat(sbushPath, ":");
        if(strcmp(tokens[i], "$PATH")!=0)
            strcat(sbushPath, tokens[i]);
        else
            strcat(sbushPath, envpath);
    }
    
    strcpy(envp[path_index], "PATH=");
    strcat(envp[path_index], sbushPath);
    
    free(sbushPath);
    free_array(tokens, token_len);
}

int cmd_set_ps1(char* command, char** ps1)
{
    int token_len = 0;
    char **tokens;
  
    trim(command);
    tokens = strtoken(command, "\"", &token_len);
    
    if(token_len != 2)
        return 0;
    
    strcpy(ps1[0], tokens[1]);

    free_array(tokens, token_len);
    return 1;
}

int cmd_pipe_init(char **tokens, int pipes, char **envp) {

    //fork a child and run that process
    int pid,status;
    pid=fork();
    if(pid==-1) {
        printf("main fork");
    } else if(pid==0) {
        //child
      runPipe(tokens,envp,pipes);
    } else {
        //parent, do nothing
	if(pid == waitpid(pid,&status,0)){ /* pick up all the dead children */ 
	    //printf("child exit successful\n");
	    ;
	}
	else{
	    printf("error in waitpid. Error no is %d\n",status);
	    exit(1);
	}
    }
    return 0;
}

int runPipe(char *tokens[],char *envp[], int pipes) 
{
    int fd[2],pid,status;
    int param_len;
    char **params = (char**)malloc(TOKEN_ARRAY_SIZE*sizeof(char*));
    trim(tokens[pipes]);
    params = strtoken(tokens[pipes], " ", &param_len);
    
    pipe(fd);
    if(pipes==0) //end case
    {  
        close(fd[0]);//close read end
        //dup2(fd[1],1);//dup write end
        cmd_binary(params,param_len,envp);
        exit(0);
    } 
    else 
    {
        pid=fork();
        if(pid==-1) 
            printf("runpipe error fork\n");
        else if(pid==0)
        {
            //child
            close(fd[0]); //close read end
            dup2(fd[1],1);
            runPipe(tokens,envp,--pipes);
        }
        else
        {
            //parent
            close(fd[1]); //close write end
            dup2(fd[0],0);
	    if(pid == waitpid(pid,&status,0)){ /* pick up all the dead children */ 
		//printf("child exit successful\n");
		;
	    }
	    else{
		printf("error in waitpid. Error no is %d\n",status);
		exit(1);
	    }

            cmd_binary(params,param_len,envp);
            exit(0);
        }
    }
    return 0;
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

char* getpath(int *index, char **envp) {

    int key=0, token_len;
    char **tokens=(char **)malloc(TOKEN_ARRAY_SIZE*sizeof(char *));
    char **temp=tokens;
    while(envp[key] != '\0')
    {
        trim(envp[key]);
        tokens = strtoken(envp[key],"=", &token_len);
        if(strcmp(tokens[0], "PATH") == 0)
        {
            *index = key;
            break;
        }
        key++;
	free_array(tokens,token_len);//free strtoken memory for not PATH env variables
    }
    free(tokens[0]);
    free(temp);
    char *res=tokens[1];
    free(tokens);
    return res;
}

int strcmp(const char *s1,const char * s2) {
    int i=0;
    for(i=0; s1[i]==s2[i]; i++) {
        if(s1[i]=='\0')
            return 0;
    }
    return s1[i]-s2[i];
}


char** strtoken(const char *s, const char *delim,int *len) {
    int i=0, j=0, k=0;
    char **tokens = (char**)malloc(TOKEN_ARRAY_SIZE*sizeof(char*));

    tokens[j] = (char*)malloc(TOKEN_SIZE*sizeof(char));

    while(s[i] != '\0') {
        if(s[i] == *delim)
        {
            tokens[j][k] = '\0';
            if(strlen(tokens[j]) > 0)
            {
                j++;
                tokens[j] = (char*)malloc(TOKEN_SIZE*sizeof(char));
            }
            k=0;
        }
        else
        {
            tokens[j][k] = s[i];
            k++;
        }
        i++;
    }
    if(strlen(tokens[j])>0)
    {
        tokens[j][k] = '\0';
        tokens[++j] = 0;
    }
    else
        tokens[j] = 0;
    
    *len=j;

    return tokens;
}

int strlen(char *s)
{
    int i = 0;
    while(s[i] != '\0')
        i++;
    return i;

}

void trim(char *s)
{
    int lead = 0, trail = 0, i=0,j=0;
    trail = strlen(s);
    char *temp = (char*)malloc(TOKEN_SIZE*sizeof(char));

    while(s[lead] == ' ')
        lead++;

    while(s[trail-1] == ' ')
        trail--;

    for(i=lead; i<trail; i++,j++)
        temp[j] = s[i];

    temp[j] = '\0';
    strcpy(s, temp);
    free(temp);
}
