#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/syscall.h>

struct dir{
    int fd;//file descriptor
    int nreads;//number of bytes read
    int bpos;//pos of cursor in buffer
    char buf[DIR_BUF_SIZE];
};

typedef struct dir * DIR;

int getdents(DIR dirp);

void *opendir(const char *name){
    DIR dirp=(DIR)malloc(sizeof(struct dir));
    dirp->fd=open(name,O_RDONLY|O_DIRECTORY);
    if(dirp->fd < 0){
	//open failed
	free(dirp);
	perror("unable to open directory\n");
	return NULL;
    }
    //open was successful
    dirp->nreads=getdents(dirp);
    //dirp->nreads=syscall_3((uint64_t)SYS_getdents,(uint64_t)dirp->fd,(uint64_t)dirp->buf,(uint64_t)DIR_BUF_SIZE);
    if(dirp->nreads==-1){
	//error reading dir entries
	perror("unable to get dirents\n");
	free(dirp);
	return NULL;
    }
    else if(dirp->nreads==0){
	//no entries in directory
	free(dirp);
	return (void *)1;
    }
    //initialize buffer position to start of the buffer
    dirp->bpos=0;
    //char *str=((struct dirent *)(dirp->buf))->d_name;
    //printf("%s\n",str);
    //printf("%d\n",sizeof(dirp->buf));
    return dirp;
}


struct dirent *readdir(void *dir){
    DIR dirp=(DIR)dir;
    struct dirent *ret;
    if(dirp->bpos < dirp->nreads){
	ret=(struct dirent *)(dirp->buf+dirp->bpos);
	dirp->bpos+=ret->d_reclen;//set bpos to next structure
	return ret;
    }
    //if all entries in buf have been read
    //then read a new bunch of entries
    dirp->bpos=0;
    dirp->nreads=getdents(dirp);
    if(dirp->nreads==-1){
	//unable to read dir entries
	perror("unable to get dirents in readdir\n");
	return NULL;

    }
    else if(dirp->nreads==0){
	//printf("end of dirents in readdir\n");
	return NULL;
    }
    else{
	ret=(struct dirent*)(dirp->buf+dirp->bpos);
	dirp->bpos+=ret->d_reclen;
	return ret;
    }    
}

int closedir(void *dir){
    DIR dirp = (DIR)dir;
    //close file descriptor
    if(close(dirp->fd)<0){
	perror("close(fd) failed\n");
	return -1;//failure
    }
    free(dirp);//deallicate stream
    return 0;//success
}


int getdents(DIR dirp){
    return (int)syscall_3((uint64_t)SYS_getdents,(uint64_t)dirp->fd,(uint64_t)dirp->buf,(uint64_t)DIR_BUF_SIZE);
}

