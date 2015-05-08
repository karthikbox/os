#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 
#include<sys/tarfs.h>
#include<errno.h>

extern void isr128();

#define LSTAR  0xC0000082
#define SFMASK 0xC0000084
#define STAR   0xC0000081
#define EFER   0xC0000080
#define LO_MASK 0x00000000FFFFFFFFul
#define HI_MASK 0xFFFFFFFF00000000ul



void init_syscall(){

  /* activate extended syscall instructions */
  /* set EFER's bit [0] SCE  */
  __asm__ __volatile__("wrmsr" : : "a"(0x1), "d"(0x0), "c"(EFER));

  /* for SYSCALL */
  /* set up STAR's bits [32-47] SYSCALL CS AND SS -> KCS,KDS */
  /* [32-16]->0x0(change if using sysret) */
  /* [16-8]->KCS->0x08 */
  /* [8-0]->KDS->0x10 */
  uint32_t hi=0x00000810;
  __asm__ __volatile__("wrmsr" : : "a"(0x0), "d"(hi), "c"(STAR));
  /* set up LSTAR with address of isr128 */
  __asm__ __volatile__("wrmsr" : : "a"(((uint64_t)&isr128) & LO_MASK), "d"((((uint64_t)&isr128) & HI_MASK)>>32), "c"(LSTAR));
  /* set up SFMASK with [0-31] with Rflags clear bit -> FL_IF */
  __asm__ __volatile__("wrmsr" : : "a"(FL_IF), "d"(0x0), "c"(SFMASK));
}


void do_yield(){
	proc->state=RUNNABLE;
	/* printf("proc -> %d -> yield syscall\n",proc->pid); */
	sched();
	
}

void do_fork(){
	/* alloc_proc finds a spot in pcb array and allocates kstack and sets sp and tf pointers and gives a pid*/
	printf("proc -> %d -> fork syscall\n",proc->pid);
	load_base(get_phys_addr((uint64_t)proc->pml4_t)); /* flush tlb */
	struct proc *p=alloc_proc();
	if(!p){
		/* p is null, alloc_proc couldnt find a spot in pcb array */
		printf("unable to fork. no space for new processs\n");
		/* put -1 in proc->tf->rax, this tells parent that fork failed */
		proc->tf->rax=-1;		/* syscall_0 returns "eax"(that's right) as uint32_t to fork(), in user program we cast to int and then check for -1 */

		/* such as kernel stack and set pcb of p to UNUSED  */
		return ;
	}

	/* copyuvm deallocates page table memory on failure, within itself */
	/* copy parent procs address space into child */
	if(!(p->pml4_t = copyuvm(proc->pml4_t))){
		/* p->pml4_t is NULL, copyuvm failed */
		printf("unable to fork. no space for new process's page tables\n");
		/* put -1 in proc->tf->rax, this tells parent that fork failed */
		proc->tf->rax=-1;		/* syscall_0 returns "eax"(that's right) as uint32_t to fork(), in user program we cast to int and then check for -1 */

		/* if any failure in cpoyuvm or copyvma, then deallocate p's resources */
		/* such as kernel stack and set pcb of p to UNUSED   */
		free_pcb(p);
		return ;

	}
	/* copy open files from parent to child */
	int fd=0;
	for(fd=0;fd<NOFILE;fd++){
		if(proc->ofile[fd]){		/* if parent's fd exists */
			p->ofile[fd]=filedup(proc->ofile[fd]); /* filedup incrs refcount of file struct parent points to */
		}
	}

	/* copy VMAs from parent to child */
	if((p->vma_head=copyvma(proc->vma_head))==NULL){
		/* copyvma failed */
		printf("unable to fork. no space for new processes vmas\n");
		proc->tf->rax=-1;
		/* copyvma takes care of freeing new procs vmas */
		/* if any failure in copyuvm or copyvma, then deallocate p's resources */
		/* such as kernel stack and set pcb of p to UNUSED and free_uvm(p->pml4)*/
		free_uvm(p->pml4_t);
		free_pcb(p);
		return ;
	}
	

	/* copy trapframe of parent to child, do not clear trapframe */
	memcpy(p->tf,proc->tf,sizeof(struct trapframe));
	/* set return of fork of child to 0 */
	p->tf->rax=0;
	/* set size of child same as parent */
	p->size=proc->size;
	/* set parent of child to proc */
	p->parent=proc;
	/* give child the same name as parent */
	strcpy(p->name,proc->name);
	/* give child same working dir as parent */
	strcpy(p->cwd,proc->cwd);
	/* return pid of child to parent */
	proc->tf->rax=p->pid;
	/* set state of child to runnable */
	p->state=RUNNABLE;
	/* flush TLB */
	load_base(get_phys_addr((uint64_t)proc->pml4_t));
}
size_t do_write(int fd, const void* bf, size_t len){
	
	size_t i=0;
	const char *buf=(const char *)bf;
	if(proc->ofile[fd]==NULL){
		return -1;				/* no local file decriptor */
	}

	/* check if the file is writable or not */	
	if(proc->ofile[fd]->writable==0){
		return -1;
	}

	/* check the file type */
	/* if file type is STDOUT, printf */
	if((proc->ofile[fd]->type==FD_STDOUT) || (proc->ofile[fd]->type==FD_STDERR)){
		for(i=0;i<len;i++){
			printf("%c",buf[i]);
		}
	}

	/* if file type is pipe, call pipewrite */
	else if(proc->ofile[fd]->type==FD_PIPE){
		return pipewrite(proc->ofile[fd]->pipe, (char *)bf, len);
	}

	return i;
} 
void do_brk(void* end_data_segment){
  
	/* printf("proc -> %d -> brk syscall\n",proc->pid); */
	struct vma *t=proc->vma_head;
	struct vma *t_stack=NULL, *t_heap=NULL;
	while(t!=NULL){
		/*traverse throug the process vma to get the heap and stack vmas */
		if(t->flags & PF_GROWSUP){
			/*if flags is PF_GROWSUP then heap*/
			t_heap=t;
		}
		else if(t->flags & PF_GROWSDOWN){
			/*if flags is PF_GROWSDOWN then stack*/
			t_stack=t;
		}
		t=t->next;
  }



	if( ((uint64_t)end_data_segment >= t_heap->end) && ((uint64_t)end_data_segment < t_stack->start) ){
		/* check end_data_segment is greater than current heap end and lesser than the current stack start*/
		t_heap->end=(uint64_t)end_data_segment + 0x1ul;
	}
	else{
		/*all other cases are invalid*/
		/*return the current break pointer*/
		end_data_segment= (void *)(t_heap->end - 0x1ul);
	}
	/* return end_data_segment in any case*/
	proc->tf->rax=(uint64_t)end_data_segment;
}


pid_t do_getpid(){
	return (pid_t)(proc->pid);
}
pid_t do_getppid(){
	if(proc->parent == NULL){
		return 0;
	}
	else{
		return (pid_t)(proc->parent->pid);
	}
}

void do_nanosleep(struct timespec *req,struct timespec *rem){
	if(req->tv_sec >= 0){
		/* sleep time is >0 secs */
		/* set state to SLEEPING */
		proc->state=SLEEPING;
		/* copy req to rem */
		rem->tv_sec=req->tv_sec;
		rem->tv_nsec=req->tv_nsec;
		/* add <proc,rem> to sleep Q */
		if(enqueue_sleep(proc,rem)==0){
			/* enqueue failed */
			/* sleep returns -1 */
			proc->tf->rax=-1;
			/* make proc RUNNING */
			proc->state=RUNNING;
			return ;
		}
		/* enqueue success */
		/* schedule next process */
		sched();
	}
}




void do_read(int fd, void* buf, size_t count){

	
	if((valid_addr((uint64_t)buf) ==0 ) || (valid_addr_range((uint64_t)buf,count)==0)){
		/* above kernbase */
		/* so return -EFAULT */
		proc->tf->rax=-EFAULT;
		return ;
	}

	/* printf("proc -> %d -> read syscall\n",proc->pid); */
	if(proc->ofile[fd]==NULL){
		proc->tf->rax=-EBADF;		/* no local file decriptor */
		return ;
	}
	if(proc->ofile[fd]->readable==0){
		proc->tf->rax=-EBADF;	/* fd is not readable */
		return ;
	}
	if(proc->ofile[fd]->type==FD_DIR){
		proc->tf->rax=-EISDIR;	/* TRYING TO READ DIR, DO GETDENTS */
		return ;
	}
	/* add check if buf is in any of VMA's */
	if(proc->ofile[fd]->type==FD_STDIN){
		/* check if foreground proc flag is set */
		if(_stdin->proc==NULL){
			/* if(_stdin->proc!=NULL){ */
			/* 	printf("_stdin Q is not free, someother process in it which is not fgproc\n"); */
			/* 	while(1); */
			/* } */
			/* add it to stdin Q */
			_stdin->proc=proc;
			_stdin->count=count;
			_stdin->buf=buf;
			/* if the terembuf is full, then we copy the requested bytes to the process and return to the process */
			if(isBufFull==1){
				do_copy();
			}
			else{
				/* since the buffer is empty, wait till the stdin has received 1 line feed */
				_stdin->proc->state=SLEEPING;
				/* schedule next process */
				sched();
				/* this process is woken up when keyboard.c receives a line feed of chars into the stdin buffer */
			}
			/* deque the proc from the stdin Q*/
			_stdin->proc=NULL;
			
		}
		else{
			/* If Set Kill This Process */
			/* there is someother proc using stdin  */
			printf("Proc -> %d Not Foreground Proc. Killing It\n",proc->pid);
			do_exit(0,proc);
		}
	}
	else if(proc->ofile[fd]->type==FD_PIPE){
		proc->tf->rax=piperead(proc->ofile[fd]->pipe, (char*)buf, count);
		return ;
	}
	else if(proc->ofile[fd]->type==FD_FILE){
		uint64_t ret=0;
		struct file *p=proc->ofile[fd];
		/* so this is a file */
		
		/* p->offset gives amount read until now*/
		/* (char *)p->addr + p->offset give next byte to be read */
		
		/* see if there is anything left to copy */
		if(p->offset >= p->size){	/* if offset is greater than file size then nothing to read */
			/* nothing left ro read */
			ret=0;			/* read 0 bytes */
		}
		else {
			/* something still left ro read */
			/* if user requested less than the file size */
			if( count <= (p->size - p->offset) ){
				/* copy requested size into the buffer */
				/* no need to load proc page table, because this is not a blocking system call */
				memcpy(buf,((char *)p->addr + p->offset),count);
				/* incr offset by number of bytes read */
				p->offset+=count;
				ret=count;
			}
			/* if user requested more than available to read */
			else if(count > (p->size - p->offset)){
				/* then copy whatevers left in to user buf */
				memcpy(buf,((char *)p->addr + p->offset),(p->size - p->offset));
				
				/* return number of bytes read */
				ret=(p->size - p->offset);

				/* incr offset by number of bytes read */
				p->offset+=(p->size - p->offset);
			}
			else{
				printf("error in read\n");
				ret=-1;
			}
		}
		/* return  */
		proc->tf->rax=ret;
		return ;
	}
	else{
		proc->tf->rax=-EBADF;		/* fd was not file, pipe, stdin */
		return ;
	}
}

void do_copy(){

	if(termbuf_head + _stdin->count <= termbuf_tail){
		/* load waiting procs page table */
		load_base(get_phys_addr((uint64_t)_stdin->proc->pml4_t));
		tss.rsp0=(uint64_t)read_kstack;
		struct proc *p=proc;
		proc=_stdin->proc;
		memcpy(_stdin->buf,termbuf_head,_stdin->count);
		proc=p;
		tss.rsp0=(uint64_t)p->kstack+KSTACKSIZE;
		/* load current procs page tables */
		load_base(get_phys_addr((uint64_t)p->pml4_t));
		termbuf_head+=_stdin->count;
		_stdin->proc->tf->rax=_stdin->count;
	}
	else{
		/* user requested more than present in buffer */
		/* load waiting procs page table */
		load_base(get_phys_addr((uint64_t)_stdin->proc->pml4_t));
		tss.rsp0=(uint64_t)read_kstack;
		struct proc *p=proc;
		proc=_stdin->proc;
		/* return till termbuf_tail */
		memcpy(_stdin->buf,termbuf_head,(termbuf_tail - termbuf_head));
		proc=p;
		tss.rsp0=(uint64_t)p->kstack+KSTACKSIZE;
		/* load current procs page tables */
		load_base(get_phys_addr((uint64_t)p->pml4_t));
		/* handle page faults */
		termbuf_head+=(termbuf_tail - termbuf_head);
		_stdin->proc->tf->rax=(termbuf_tail - termbuf_head);
	}
	if(termbuf_head==termbuf_tail){
		/* all termbuf has been read */
		/* restart from begining */
		termbuf_head=termbuf_tail=termbuf;
		/* termbuf is empty */
		isBufFull=0;
	}
}


void do_pipe(int *fd_arr){
	printf("proc -> %d -> pipe syscall\n",proc->pid);
	/* allocate 2 FILE structs */
	struct file *rf,*wf;
	int fd0, fd1;
	if(pipealloc(&rf,&wf) < 0){
		proc->tf->rax=-1;
		return;
	}
	fd0=-1;
	/* put those pounters in local pcb ofiles fd table */
	if((fd0=fdalloc(rf)) < 0 || (fd1=fdalloc(wf)) < 0){
		/* fd alloc failed */
		printf("unable to allocate local fd\n");
		if(fd0 >= 0){
			proc->ofile[fd0]=NULL;
		}
		fileclose(rf);
		fileclose(wf);
		proc->tf->rax=-1;
		return ;
	}
	/* put index of local pcb ofiles fd table in fd_arr[0] and fd_arr[1] */
	fd_arr[0]=fd0;
	fd_arr[1]=fd1;
	proc->tf->rax=0;
	return ;
}

void do_close(int fd){

	/* decrease the reference count of the file by calling fileclose */
	fileclose(proc->ofile[fd]);

	/* make the file entry of the process to 0 meaning free */
	proc->ofile[fd]=0;

	/* store the return value */
	proc->tf->rax=0;
}

int do_dup(int old_fd){
	if( (old_fd < 0) || (old_fd >= NOFILE))
		return -1;
	
	/* allocate new local fd and make it point to old fd's file struct */
	int fd;
	if((fd=fdalloc(proc->ofile[old_fd])) ==-1){
		return -1;				/* no available local fd */
	}
	filedup(proc->ofile[old_fd]); /* increment ref count file struct  */
	return fd;
}

int do_dup2(int old_fd, int new_fd){

	if( (old_fd < 0) || (old_fd >= NOFILE))
		return -1;

	if( (new_fd < 0) || (new_fd >= NOFILE))
		return -1;


	/* check if new fd is pointing to some file struct */
	if(proc->ofile[new_fd]!=NULL){
		/* unlink new_fd from this file struct */
		fileclose(proc->ofile[new_fd]);;
		proc->ofile[new_fd]=NULL;
	}
	/* new fd is empty */
	/* make new_fd point to old_fd struct */
	proc->ofile[new_fd]=proc->ofile[old_fd];
	filedup(proc->ofile[new_fd]); /* increment ref count file struct  */
	return new_fd;
}


char *do_getcwd(char *buf,size_t size){
	/* check if buf is a valid addr */
	/* if it falls in any of the proc's vmas */
	if(valid_addr((uint64_t)buf)==0){
		/* not a valid addr */
		/* kill proc?? */
		do_exit(0,proc);
		return NULL;			/* will never return NULL */		
	}
	else{
		/* valid ptr */
		if((strlen(proc->cwd)+1) > size ){
			/* cwd doesnt fit in buf */
			return NULL;
		}		   
		else{
			/* cwd fits in user buf */
			strcpy(buf,proc->cwd);
			return buf;
		}
	}
}

int do_chdir(const char *path){
	/* On success returns 0, on failure returns -1 */
	uint64_t *tarfs_file;
	/* copy path to kernel memory */
	char *kpath = (char*)kmalloc(NCHARS*sizeof(char));
	strcpy(kpath, (char *)path);
	/* get the absolute path  */
	if(get_absolute_path(kpath) == NULL){
		kfree(kpath);
		return -1;
	}
	/* if it is not root directory */
	if(strcmp(kpath, "")){
		/* check if the path is valid or not */
		tarfs_file = tarfs_get_file(kpath,DIRTYPE);
		/* change proc->cwd, if path is valid */
		if(tarfs_file == NULL){
			kfree(kpath);
			return -1;
		}
	}
	
	strcpy(proc->cwd, "/");
	strcat(proc->cwd, kpath);
	kfree(kpath);
	return 0;
}

int do_open(char *buf, uint64_t flags){
	uint64_t *tarfs_file;
	/* copy path to kernel memory */
	if((valid_addr((uint64_t)buf)==0) || (valid_addr_range((uint64_t)buf,strlen(buf)+1)==0)){
		return -EFAULT;
	}
	if((strlen(buf)+1) > NCHARS){
		return -ENAMETOOLONG;
	}
		
	char *kpath = (char*)kmalloc(NCHARS*sizeof(char));
	if(kpath==NULL)
		return -ENOMEM;
	strcpy(kpath, (char *)buf);	
	
	char type;
	int lfd;

	if(get_absolute_path(kpath) == NULL){
		kfree(kpath);
		return -ENAMETOOLONG;
	}


	if((flags == (O_RDONLY|O_DIRECTORY))){
		/* is directory */
		type=DIRTYPE;
			/* lookup the path */
		/* get the absolute path  */
		
		/* if it is not root directory */
		if(strcmp(kpath, "")){
			/* check if the path is valid or not */
			tarfs_file = tarfs_get_file(kpath,type);
			/* change proc->cwd, if path is valid */
			if(tarfs_file == NULL){
				kfree(kpath);
				return -ENOENT;		/* N0 SUCH FILE */
			}
		}
		
		else{
			/* if it is a root dir */
			/* points to first tarfs entry */
			tarfs_file=(uint64_t *)&_binary_tarfs_start;
		}
		
		/* create global file structures */
		struct file *fd;
		/* alloc local fd to point global file struct */
		if((fd=filealloc())<0){
			kfree(kpath);
			return -ENFILE;		/* SYSTEM LIMIT ON OPEN FILES */
		}
		/* initialise global file struct */
		fd->type=FD_DIR;
		fd->readable=1;
		fd->writable=0;
		fd->offset=0;
		fd->addr=(uint64_t *)tarfs_file;
		memset1((char *)fd->inode_name,0,sizeof(char)*NCHARS);
		strcpy(fd->inode_name,kpath);
		/* return local fd */
		if((lfd=fdalloc(fd))<0){
			kfree(kpath);
			fileclose(fd);
			return -EMFILE;			/* PROCESS LIMIT ON OPEN FILES */
		}
		kfree(kpath);
		return lfd;
		
	}
	else if(flags == O_RDONLY){
		type=REGTYPE;
		/* get_virtual_path always returns with a / appended */
		/* remove this slash as, files are not terminated witha / in tarfs */
		int len=strlen(kpath);
		kpath[len-1]='\0';
		/* get file from tarfs */
		tarfs_file=tarfs_get_file(kpath,type);
		/* if it is  null return -ENOFILE */
		if(tarfs_file==NULL){
			kfree(kpath);
			return -ENOENT;		/* no such file */
		}
		/* if it is not null continue */

		/* once we have begining of file in tarfs */		
		/* allocate a global file struct */
		struct file *fd;
		if((fd=filealloc()) < 0){
			/* failed->return -ENFILE */
			kfree(kpath);
			return  -ENFILE;
		}
		/* allocate local file struct */
		if((lfd=fdalloc(fd)) < 0 ){
			/* failed-> return -EMFILE */
			kfree(kpath);
			fileclose(fd);
			return -EMFILE;
		}
		/* initialize file struct with FD_FILE  */
		fd->type=FD_FILE;
		/* set readable */
		fd->readable=1;
		fd->writable=0;
		/* set addr to addr returned from tarfs */
		fd->addr=(uint64_t *)tarfs_file;
		/* set offset to 0 */
		fd->offset=0x0ul;
		/* set size of file */
		/* convert form octol to decimal */
		fd->size=oct_to_dec(((struct posix_header_ustar *)tarfs_file -1 )->size);
		/* clear contents of inode_name from the file struct */
		memset1((char *)fd->inode_name,0,sizeof(char)*NCHARS);
		/* copy file path into inode_name */
		strcpy(fd->inode_name,kpath);
		/* return local fd */
		kfree(kpath);
		return lfd;
	}
	else{
		return -EACCES;			/* requested mode is not allowed */
		/* only read only and o_directory is allowed */
	}
	
}

int do_getdents(int fd, char* buf, size_t len){

	/* sanity check */
	if(fd<0 || fd>=NOFILE || proc->ofile[fd]==NULL || proc->ofile[fd]->type!=FD_DIR || !valid_addr((uint64_t)buf) ){
		return -1;
	}
	int mustBeEmpty=0;
	int nreads=0;
	if(strcmp(proc->ofile[fd]->inode_name,"")==0){
		/* root directory */
		mustBeEmpty=1;
		nreads=add_root(proc->ofile[fd],mustBeEmpty,buf,len);
	}
	else{
		/* non root directory */
		nreads=add_non_root(proc->ofile[fd],buf,len);
	}
	return nreads;
}


int add_root(struct file *fd,int mustBeEmpty,char *buf,size_t len){
	char **tokens;	
	int token_len;
	uint64_t *p_e= (uint64_t *)&_binary_tarfs_end;
	int nreads=0;
	struct posix_header_ustar *p;
	struct dirent *dirent;
	while((((char *)fd->addr+fd->offset) < (char *)p_e)){
		p=(struct posix_header_ustar *)((char *)fd->addr+fd->offset);
		if(strlen(p->name)==0)
			break;
		printf("%p\n",(fd->offset+(char *)fd->addr));
		/* tokenize the string in addr+offset , tarfs header */
		tokens=strtoken(p->name,"/",&token_len );
		/* if tk_len =2, and if tk[1]="" , since root directory*/
		if((token_len == mustBeEmpty) && (tokens[mustBeEmpty]==NULL)){
			/* root dir entry */
			/* form linux struct */
			/* check if new entry fits in buf */
			if((buf+nreads) + sizeof(struct dirent) < (buf+len)){
				/* space available for new dentry */

				/* initialize dirent  */
				dirent=(struct dirent *)(buf+nreads);
				dirent->d_ino=999; /* dummy */
				dirent->d_reclen=sizeof(struct dirent);
				dirent->d_off=sizeof(struct dirent);
				strcpy(dirent->d_name,tokens[mustBeEmpty-1]);
				nreads+=sizeof(struct dirent);
				
			}
			else{
				/* no space for dirent */
				/*  return, free before return  */
				free_array(tokens,token_len);
				return nreads;
			}
		}
		fd->offset+=(sizeof(struct posix_header_ustar)+round_up(oct_to_dec(p->size),512));
		free_array(tokens,token_len);
	}
	return nreads;
	

}

int add_non_root(struct file *fd,char *buf,size_t len){
	char **tokens;	
	char prev[NCHARS];
	memset1((char *)prev,0,NCHARS);
	int token_len;
	uint64_t *p_e= (uint64_t *)&_binary_tarfs_end;
	int nreads=0;
	struct posix_header_ustar *p;
	struct dirent *dirent;
	while((((char *)fd->addr+fd->offset) < (char *)p_e)){
		p=(struct posix_header_ustar *)((char *)fd->addr+fd->offset);
		if(strlen(p->name)==0)
			break;

		/* tokenize the string in addr+offset , tarfs header */
		tokens=strtoken(p->name,"/",&token_len );
		/* if tk_len =2, and if tk[1]="" , since root directory*/

		if((token_len >= 2) && (tokens[0]!=NULL) && (tokens[1]!=NULL)){
			strcat(tokens[0],"/");
			for(int i=1;i<token_len-1;i++){
				strcat(tokens[0],tokens[i]);
				strcat(tokens[0],"/");
			}
			if((strcmp(tokens[0],fd->inode_name)==0) && (strcmp(tokens[token_len-1],prev)!=0)){
				/* root dir entry */
				/* form linux struct */
				/* check if new entry fits in buf */
				if((buf+nreads) + sizeof(struct dirent) < (buf+len)){
					/* space available for new dentry */
					
					/* initialize dirent  */
					dirent=(struct dirent *)(buf+nreads);
					dirent->d_ino=999; /* dummy */
					dirent->d_reclen=sizeof(struct dirent);
					dirent->d_off=sizeof(struct dirent);
					strcpy(dirent->d_name,tokens[token_len-1]);
					nreads+=sizeof(struct dirent);
					strcpy(prev,tokens[token_len-1]);
				}
				else{
					/* no space for dirent */
					/*  return, free before return  */
					free_array(tokens,token_len);
					return nreads;
				}
			}
		}
		fd->offset+=(sizeof(struct posix_header_ustar)+round_up(oct_to_dec(p->size),512));
		free_array(tokens,token_len);
	}
	return nreads;
}


int64_t do_lseek(int fd,int64_t off,int whence){
	if((fd<0) || (fd >= NOFILE) ){
		return -EBADF;
	}
	/* check if fd exeists */
	if(proc->ofile[fd]==NULL){
		return -EBADF;
	}

	if(proc->ofile[fd]->type==FD_PIPE){
		return -ESPIPE;
	}
	/* p points to file struct */
	struct file *p=proc->ofile[fd];
	if(whence==SEEK_SET){
		p->offset=off;			/* set file offset to given off*/
		return p->offset;
	}
	else if(whence==SEEK_CUR){
		p->offset=p->offset+off; /* the offset is set to its current location plus offset bytes */
		return p->offset;		
	}
	else if(whence==SEEK_END){
		p->offset=p->size+off;	/* the offset is set to the file size plus offset bytes */
		return p->offset;
	}
	else{
		/* invalid whence */
		return -EINVAL;
	}
	
}
