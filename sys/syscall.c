#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 
#include<sys/tarfs.h>

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

void do_exit(int status, struct proc *p){

	printf("proc -> %d -> exit syscall\n",proc->pid);
	/* free vmas free_vma_list(head) */
	free_vma_list(&(p->vma_head));
	
	/* free process page tables free_uvm(pml4_t) */
	free_uvm(p->pml4_t);
	
	/* update waitpid Q */
	update_waitpid_queue(p);

	/* if current process is present in stdin Q, and we decide to kill process */
	/* then dequeu proc from stdin Q first */
	/* this use case will mostly occur during page fault in do_copy */
	if(_stdin->proc == p){
		/* p is enqued in stdin Q */
		/* dequue from stdin Q */
		_stdin->proc=NULL;
	}
	/* free pcb */
	free_pcb(p);
	
	/* call the sched */
	sched();
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
	printf("proc -> %d -> sleep syscall\n",proc->pid);
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

void do_waitpid(pid_t pid, int* status, int options){
	
	printf("proc -> %d -> waitpid syscall\n",proc->pid);
	/* change the process state to SLEEPING */
	proc->state=SLEEPING;
	/* add the process to a waitpid Q */
	if(enqueue_waitpid(proc,pid)==0){
		/* enqueue failed */
		/* waitpid returns -1 */
		proc->tf->rax=-1;
		/* make process running */
		proc->state=RUNNING;
    return;
	}
	/* enqueue success */
	/* schedule another process */
	sched();
}

void do_read(int fd, void* buf, size_t count){
	/* printf("proc -> %d -> read syscall\n",proc->pid); */
	if(proc->ofile[fd]==NULL){
		proc->tf->rax=-1;		/* no local file decriptor */
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

int valid_addr(uint64_t addr){
	return 1;
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
	char *kpath = (char*)kmalloc(NCHARS*sizeof(char));
	strcpy(kpath, (char *)buf);	
	char type;
	int lfd;
	if(flags & O_DIRECTORY){
		/* is directory */
		type=DIRTYPE;
	}
	/* lookup the path */
	/* get the absolute path  */
	if(get_absolute_path(kpath) == NULL){
		kfree(kpath);
		return -1;
	}

	/* if it is not root directory */
	if(strcmp(kpath, "")){
		/* check if the path is valid or not */
		tarfs_file = tarfs_get_file(kpath,type);
		/* change proc->cwd, if path is valid */
		if(tarfs_file == NULL){
			kfree(kpath);
			return -1;
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
		return -1;
	}
	/* initialise global file struct */
	fd->type=FD_DIR;
	fd->readable=1;
	fd->writable=0;
	fd->offset=0;
	fd->addr=(uint64_t *)tarfs_file;
	/* return local fd */
	if((lfd=fdalloc(fd)<0)){
		kfree(kpath);
		fileclose(fd);
		return -1;
	}
	return lfd;
}

int do_getdents(int fd, char* buf, size_t len){

	/* sanity check */
	if(fd<0 || fd>=NOFILE || proc->ofile[fd]==NULL || proc->ofile[fd]->type!=FD_DIR || valid_addr((uint64_t *)buf)==0 ){
		return -1;
	}
}
