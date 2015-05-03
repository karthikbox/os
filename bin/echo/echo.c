#include <stdio.h>
#include <stdlib.h>

int is_envp(char *env, char* envp[]);

int main(int argc, char* argv[], char* envp[]){
	int i;
   	if(argc==1){
		printf("\n");
		return 0;
	}

	for(i=1;i<argc;i++){
		
		/* check if it is an environment variable */
		if(argv[i][0]=='$'){
			if(is_envp((char *)argv[i]+1, envp)==0){
				printf("%s: Undefined variable\n", (char *)argv[i]+1);
				return 0;
			}
		}
		else{
			printf("%s ",argv[i]);
		}

	}
	printf("\n");
	return 0;
}

int is_envp(char *env, char* envp[]){
	int i=0,j=0;
	for(;envp[i]!=NULL;i++){
		j=0;
		while(envp[i][j] != '='){
			if(envp[i][j] != env[j]){
				break;
			}
			j++;
		}
		if(envp[i][j] == '=' && env[j]=='\0'){
			printf("%s ", (char *)envp[i] + j+1);
			return 1;
		}
	}
	return 0;
}
