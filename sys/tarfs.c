#include<sys/tarfs.h>
#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/defs.h>
#include<sys/process.h>
#include<sys/memory.h>

uint64_t * tarfs_get_file(char name[], char type){
	struct posix_header_ustar *p= (struct posix_header_ustar *)&_binary_tarfs_start;
	struct posix_header_ustar *p_e= (struct posix_header_ustar *)&_binary_tarfs_end;
	
	while(p<p_e && !(strlen(p->name)==0)){
		/* printf("cur name->%s, lookup name->%s, cur location->%p\n",p->name,name,p); */
		if(strcmp(p->name,name)==0){
			if(*p->typeflag==type){
				/* this is the required file name */
				return (uint64_t *)(p+1);			/* return pointer to start of ELF binary */
			}
			else{
				return NULL;
			}
 
		}
		else{
			/* goto next file header by adding size of header and header->size */
			/* header->size should be rounded up next 512 multiple */
			//printf("%s->%u",oct_to_dec("00000 0000 0000"))
			p=(struct posix_header_ustar *)((char *)p+sizeof(struct posix_header_ustar ) + round_up(oct_to_dec(p->size),512));
		}
	}
	/* no file with that name */
	return NULL;

	
}


uint64_t oct_to_dec(char str[]){
	/* char[12] str */
	//return (uint64_t)58960;
	int i,prod=1;
	uint64_t sum=0;
	for(i=10;i>=0;i--){
		sum+=prod*(str[i]-'0');
		prod*=8;
	}
	return sum;
}


uint64_t round_up(uint64_t sz,uint64_t mul){
	if(sz%mul==0){
		return sz;
	}
	else{
		return sz + mul - (sz%mul);
	}
}

uint64_t * get_absolute_path(char *path){

	char absolute_path[100];
	char **tokens;
	int token_len;
	int i,j,len;
	i=j=len=0;

	/* return 1 on success, 0 on failure */
	/* check if it is absolute or not */
	/* if absolute remove the first slash */
	memset1((char*)absolute_path, 0, 100);
	if(path[0] == '/'){
		strcpy(absolute_path, path+1);
	}
	/* if relative append cwd to path */
	else{
		strcpy(absolute_path,(char*)(proc->cwd) + 1) ;
		/* if(strcmp(proc->cwd, "/")==0){ */
		/* 	strcat(absolute_path, "/"); */
		/* } */
		strcat(absolute_path, path);
	}
	
	/* get all the directories in the path */
	tokens=strtoken(absolute_path, "/", &token_len);
	
	/* printf("absolutepath -> %s\n",absolute_path); */
	/* printf("token len -> %d\n", token_len); */

	for(i=0;i<token_len;i++){
		
		/* directory is '.' skip */
		if(!strcmp(tokens[i],".")){
			continue;
		}
		/* directory is '..' move backwards */
		else if(!strcmp(tokens[i],"..")){
			j-=1;
			/* this is the case when you are going below the root */
			if(j<0){
				j=0;
			}
		}
		else{
			strcpy(tokens[j],tokens[i]);
			j++;
		}
	}
	/* clear absolute_path */
	memset1(absolute_path, 0, strlen(absolute_path));
	
	/* if it is root folder, return empty string */
	if(j==0){
		strcpy(path, "");
	}
	/* concatenate all the paths */
	for(i=0;i<j;i++){
		strcat(absolute_path, tokens[i]);
		strcat(absolute_path, "/");
	}
	
	len=strlen(absolute_path);
	if(len > NCHARS){
		free_array(tokens,token_len);
		return NULL;
	}
	
	/* terminate the string */
	absolute_path[len]='\0';
	strcpy(path, absolute_path);
	free_array(tokens,token_len);
	return (uint64_t*)path;
}

char** strtoken(const char *s, const char *delim,int *len) {
    int i=0, j=0, k=0;
    char **tokens = (char**)kmalloc(NCHARS*sizeof(char*));

    tokens[j] = (char*)kmalloc(NCHARS*sizeof(char));

    while(s[i] != '\0') {
        if(s[i] == *delim)
        {
            tokens[j][k] = '\0';
            if(strlen(tokens[j]) > 0)
            {
                j++;
                tokens[j] = (char*)kmalloc(NCHARS*sizeof(char));
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

void free_array(char **tokens,int len) {
    int i;
    for(i=0; i<len; i++)
        kfree(tokens[i]);
    kfree(tokens);
}
