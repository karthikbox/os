
#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/tarfs.h>
#include<sys/syscall.h>
#include<errno.h>

extern void trapret();
extern int exec(char *path,char **argv,char **envp);
struct proc *initproc;
struct proc *proc;
uint32_t proc_count=1;			/* this is the pid counter. starts from 1. */
struct cpu cpu;


inline uint64_t get_virt_addr(uint64_t x){
	return KERNBASE+x;
}

inline uint64_t get_phys_addr(uint64_t x){
	return x-KERNBASE;
}

inline uint64_t SEG_KCS(){
 	return (uint64_t)(GDT_CS | P | DPL0 | L);  /*** kernel code segment descriptor ***/
}

inline uint64_t SEG_KDS(){
 	return (uint64_t)(GDT_DS | P | W | DPL0);  /*** kernel data segment descriptor ***/
}

inline uint64_t SEG_UCS(){
 	return (uint64_t)(GDT_CS | P | DPL3 | L);  /*** USER code segment descriptor ***/
}

inline uint64_t SEG_UDS(){
 	return (uint64_t)(GDT_DS | P | W | DPL3);  /*** USER data segment descriptor ***/
}



struct{
	struct proc proc[NPROC];
}ptable;

/* gloable file table */
struct {
	struct file file[NFILE];
}ftable;


pml4 *load_kern_vm(){
	/* allocate a pml4 entry. load kernel vm into pml4 entry and return it */
	pml4 *pml4_t;
	if(!(pml4_t=(pml4 *)kmalloc(FRAME_SIZE))){
		return NULL;
	}
	memset1((char *)pml4_t,0,FRAME_SIZE);
	
	/* put kernel space which is marked RW for only supervisor into process pml4 table */
	/* pml4_base is a physical addr of 0x100000. */
	/* to derefernce pml4 entries. conver the physical address into virt addr  and then dereference it*/
	
	pml4_t->m_entries[511]=((pml4 *)get_virt_addr((uint64_t)pml4_base))->m_entries[511];
	return pml4_t;
	
}

/* load initcode.S into address 0 of pml4 */
void inituvm(pml4 *pml4_t, char *star,uint64_t sz){
	char *mem;
	if(sz >= FRAME_SIZE){
		/* printf("inituvm: input sz more than a page\n"); */
		;
	}
	mem=(char *)kmalloc(FRAME_SIZE);
	memset1(mem,0,FRAME_SIZE);
	
}

/* creates the first process */
void userinit(){
	struct proc *p;
	/* printf("entered userinit\n"); */
	/* allocate memory for stdin wait queue */
	_stdin = (struct read_proc*)kmalloc(sizeof(struct read_proc));
	memset1((char *)_stdin,0,sizeof(struct read_proc));
	memset1((char *)ptable.proc,0,sizeof(ptable));
	p=alloc_proc();
	initproc=p;
	fgproc=initproc;
	if(!(p->pml4_t=load_kern_vm())){
		/* panic code goes here*/
		printf("Kernel Panic: Unable To Create Initproc\n");
	}
	//inituvm(p->pml4_t,binary_initcode_start,binary_initcode_size);
	p->size=FRAME_SIZE;
	/* clear the trapframe. this is only done for fist process */
	memset1((char *)p->tf,0,sizeof(struct trapframe));
	/* p->tf->cs=0x8/\* SEG_KCS() *\/;			/\* kernel code segment *\/ */
	/* p->tf->ss=0x10/\* SEG_KDS() *\/;			/\* kernel data segment *\/ */
	p->tf->cs=0x1B;			/* user code segment */
	p->tf->ss=0x23;			/* user data segment */
	p->tf->rflags=FL_IF;			/* enable interrupts */
	
	/* rsp will be sey in exec */
	/* p->tf->rsp=FRAME_SIZE; /\* where does the user stack star from in the proc vm *\/ */

	/* rip will be set in exec */
	/* p->tf->rip=0;		   /\* begining of initcode.S *\/ */

	strcpy(p->name,"initcode");	/* p->name has a size of 32 bytes */
	p->state=RUNNABLE;
	
	
	/* printf("calling exec\n"); */
	
	/* call exec */
	proc=p;
	/* set cwd of initproc to root '/' */
	strcpy(proc->cwd,"/");

	char *a="bin/hello";
	char *argv[3];
	argv[0]=a;argv[1]="os/sbunix";argv[2]=NULL;
	char *envp[7];
	envp[0]="PATH=/bin";
	envp[1]="envp1";
	envp[2]="envp2";
	envp[3]="envp3";
	envp[4]="envp4";
	envp[5]="envp5";
	envp[6]=NULL;
	cli();

	exec("bin/hello",argv,envp);
	//p->tf->cs=(SEG_);
	/* ltr(0x2Bu); */
	/* ltr 0x2B   with RPL of 3 for user??? */
	ltr(0x2Bu);

	/* set WP bit in cr0 */
	set_wp_bit();


	/* clear ftable */
	memset1((char *)ftable.file,0,sizeof(struct file)*NFILE);
	/* clear file descriptor table of proc,i.e initproc */
	int fd=0;
	for(fd=0;fd<NOFILE;fd++){
		proc->ofile[fd]=NULL;
	}
	struct file *fp=NULL;
	/* give initproc STDIN, STDOUT,STDERR*/
	proc->ofile[STDIN]=filealloc();
	if(proc->ofile[STDIN]==NULL){
		printf("Kernel Panic, STDIN fiel struct not allocd for initproc\n");
		/* panic. kill proc */
	}
	fp=proc->ofile[STDIN];
	fp->type=FD_STDIN;
	fp->readable=1;	/* mark readable */
	fp->writable=0;	/* mark not writable */

	proc->ofile[STDOUT]=filealloc();
	if(proc->ofile[STDOUT]==NULL){
		printf("Kernel Panic: STDOUT file struct not allocd for initproc\n");
		/* panic. kill proc */
	}
	fp=proc->ofile[STDOUT];
	fp->type=FD_STDOUT;
	fp->readable=0;	/* mark NOT readable */
	fp->writable=1;	/* mark writable */


	proc->ofile[STDERR]=filealloc();
	if(proc->ofile[STDERR]==NULL){
		printf("Kernel Panic: STDERR fiel struct not allocd for initproc\n");
		/* panic. kill proc */
	}
	fp=proc->ofile[STDERR];
	fp->type=FD_STDERR;
	fp->readable=0;	/* mark NOT readable */
	fp->writable=1;	/* mark  writable */


	/* initialize sleep_head and sleep_tail to NULL */
	init_sleep_queue();
	init_waitpid_queue();
	init_stdin_queue();

	/* initialize inodes */
	/* init_inodes(); */
	/* printf writes to <1MB mem region. Now user page tables are loaded. We cannot access <1MB since we did not map that region into user process < 1MB VM aread */
	/* printf("calling scheduler\n"); */
	scheduler();
}


/* creates a process, any time */
struct proc * alloc_proc(){
	struct proc *p;
	char *sp;
	for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
		if(p->state==UNUSED){
			p->state=EMBRYO;
			p->pid=proc_count++;
			/* allocate kernel stack */
			if(!(p->kstack=(char *)kmalloc(KSTACKSIZE))){ 
				/* KSTACKSIZE IS 4096B*/
				p->state=UNUSED;
				return NULL;
			}
			sp=p->kstack+KSTACKSIZE; /* points to end of stack */
			/* make space for trapframe */
			sp-=sizeof(struct trapframe);
			p->tf=(struct trapframe *)sp;
			/* set up new context to start executing at forkret, ehich returns to trapret */
			sp-=8;
			*(uint64_t *)sp=(uint64_t)trapret;
			sp-=(sizeof(struct context));
			sp+=8;
			p->context=(struct context *)sp;
			/* clear contents of ofile array */
			memset1((char *)p->ofile,0,sizeof(struct file *)*NOFILE);
			/* clear contents of cwd */
			memset1((char *)p->cwd,0,sizeof(char)*NCHARS);
			return p;
		}
	}
	return NULL;
}

void forkret(){
	static int first =1;
	if(first){
		/* some initialization, if necessary */
		first=0;
	}
}


void scheduler(){


	struct proc *p;
	
	while(1){
		/* set interrupts ??? */
		for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
			if(p->state!=RUNNABLE){
				continue;
			}
			/* switch to this process. */
			proc=p;
			switchuvm(p);
			p->state=RUNNING;
			swtch(&cpu.scheduler,p->context);
			/* __asm__ volatile( */
			/* 		"movq %0,%%rsp;" */
			/* 		: */
			/* 		:"r"((char *)(proc->tf) - 8) */
			/* 		: */
			/* 				 ); */

			/* __asm__ volatile("retq;"); */
			
		}
	}

}

void sched(){
	swtch(&proc->context,cpu.scheduler);
}

void swtch(struct context **cpu,struct context *new ){
	__asm__ __volatile__("	pushq %%rbx;"
				  "pushq %%rbp;"
				  "pushq %%r12;"
				  "pushq %%r13;"
				  "pushq %%r14;"
				  "pushq %%r15;"
				  "movq %%rsp,(%0);"
				  "movq %1,%%rsp;"
				  "popq %%r15;"
				  "popq %%r14;"
				  "popq %%r13;"
				  "popq %%r12;"
				  "popq %%rbp;"
				  "popq %%rbx;"
				  "retq;"
				  :
				  :"r"(cpu),"r"(new)
				  );
	/* push callee save regs */
	/* pushq %%rbx; */
	/* pushq %%rbp; */
	/* pushq %%r11; */
	/* pushq %%r12; */
	/* pushq %%r13; */
	/* pushq %%r14; */
	/* pushq %%r15; */
	/* movq rsp to *cur */
	/* movq %%rsp,%0; */
	/* movq *new to rsp */
	/* movq %1,%%rsp */
	/* pop calle save regs */
	/* popq %%r15; */
	/* popq %%r14; */
	/* popq %%r13; */
	/* popq %%r12; */
	/* popq %%r11; */
	/* popq %%rbp; */
	/* popq %%rbx; */
	/* retq */
}

void switchkvm(){
	load_base(get_phys_addr((uint64_t)pml4_base));
}

void switchuvm(struct proc *p){
	/* cli(); */
	/* load gdt[5].ss0= */
	tss.rsp0=(uint64_t)p->kstack+KSTACKSIZE;
	/* ltr(TSS_ADDRESSS); */
	if(!p->pml4_t){
		/* printf("no page tables allocated for this process. They should have been. Fatal.\n"); */
		/* PANIC */
		;
	}
	load_base(get_phys_addr((uint64_t)p->pml4_t)); /* load process page tables */
	/* sti(); */
	
	/* printf("tss.rsp0 -> %p\n",tss.rsp0); */
	
}

inline void cli(){
	__asm__ __volatile__("cli");
}

inline void sti(){
	__asm__ __volatile__("sti");
}

inline void ltr(uint16_t v){
	__asm__ __volatile__("ltr %0;"
						 :
						 :"r"(v)
						 :
						 );
}


void free_vma_list(struct vma **p){
	/* iterate throught he vma's */
	struct vma *t=*p;
	if(!t)
		return;
	struct vma *n=t->next;
	if(n){
		while(n){
		/* no not null, next vma exists */
			kfree(t);
			t=n;
			n=n->next;
		}
	}
	kfree(t);
	/* make vma_head =null */
	*p=NULL;

}


struct file * filedup(struct file *f){
	if(f->ref < 1){
		/* printf("file struct ref count less than 1\n. filedup error\nproc pid->%d\n",proc->pid); */
		/* panic(filedup) */
		/* kill proc */
		;
	}
	f->ref++;
	return f;
}


int enqueue_sleep(struct proc *p,struct timespec *rem){
	/* returns 0 on failure and 1 on success */
	struct sleep_entry *t=(struct sleep_entry *)kmalloc(sizeof(struct sleep_entry));
	t->proc=p;
	t->rem.tv_sec=rem->tv_sec;
	t->rem.tv_nsec=rem->tv_nsec;
	t->next=NULL;
	if(t==NULL){
		/* printf("unable to allocate memory...enqueue_sleep failed\n"); */
		return 0;
	}
	if(sleep_head == NULL ){
		/* Q is empty */	
		sleep_head=sleep_tail=t;
	}
	else{
		/* Q is not empty */
		sleep_tail->next=t;
		sleep_tail=t;
	}
	return 1;
}

void dequeue_sleep(struct sleep_entry *p){
	/* remove given node from Q */
	if(p==sleep_head){
		/* first node */
		if(sleep_head->next==NULL){
			/* if only one node in Q */
			sleep_head=sleep_tail=NULL;
		}
		else{
			/* move head to next node */
			sleep_head=p->next;
		}
		/* free cur node */
		kfree(p);
	}
	else{
		/* middle or end node */
		struct sleep_entry *prev=NULL, *cur=sleep_head;
		for(;cur!=NULL;prev=cur,cur=cur->next){
			if(cur==p){
				/* found node */
				prev->next=cur->next;
				if(cur->next==NULL){
					sleep_tail=prev;
				}
				kfree(cur);
			}
		}
	}
	
}

void update_sleep_queue(){
	struct sleep_entry *p=sleep_head;
	for(;p!=NULL;p=p->next){
		if(p->rem.tv_sec==0){
			
			/* proc's sleep timer has expired */
			/* make process RUNNABLE */
			p->proc->state=RUNNABLE;
			/* remove from sleep Q */
			dequeue_sleep(p);
			/* set rax of p to 0, since it has successfully completed sleep period */
			p->proc->tf->rax=0;
		}
		else{
			/* decrement rem time of every proc in sleep Q */
			p->rem.tv_sec-=1;			
			p->rem.tv_nsec=0L; 
		}
	}
}

void init_sleep_queue(){
	/* initialize head and tail of sleep Q */
	sleep_head=NULL;
	sleep_tail=NULL;	
}

void init_waitpid_queue(){
  waitpid_head=NULL;
  waitpid_tail=NULL;
}

int enqueue_waitpid(struct proc *p, int pid){
	struct waitpid_entry *t=(struct waitpid_entry *)kmalloc(sizeof(struct waitpid_entry));
	t->parent_proc=p;
	t->pid=pid;
	t->next=NULL;
	if(t==NULL){
		/* printf("unable to allocate memory..enqueue_waitpid failed\n"); */
		return 0;
	}
	if(waitpid_head==NULL){
		/* Q is empty */
		waitpid_head=waitpid_tail=t;
	}
	else{
		/* Q is not empty */
		waitpid_tail->next=t;
		waitpid_tail=t;
	}
	return 1;
}

void update_waitpid_queue(struct proc *p){
  /* traverse through the queue */
	struct waitpid_entry *t=waitpid_head;
	for(;t!=NULL;t=t->next){
		/* compare the current process' parents pid with the process ids in the queue */
		if((t->pid==-1) || (t->pid==p->pid)){
			if(p->parent->pid == t->parent_proc->pid){
				
				/* remove from waitpid Q */
				dequeue_waitpid(t);
				
				/* if the pids match, return the pid of the current process */
				t->parent_proc->tf->rax=p->pid;
				t->parent_proc->state=RUNNABLE;
			}
		}
		else if(t->parent_proc->pid==p->pid){
			
			/* remove from waitpid Q */
				dequeue_waitpid(t);
		}
	}
}

void dequeue_waitpid(struct waitpid_entry *p){
  /* remove given node from Q */
  if(p==waitpid_head){
    /* first node */
    if(waitpid_head->next==NULL){
      /* if only one node in Q */
      waitpid_head=waitpid_tail=NULL;
    }
    else{
      /* move head to next node */
      waitpid_head=p->next;
    }
    /* free cur node */
    kfree(p);
  }
  else{
    /* middle or end node */
    struct waitpid_entry *prev=NULL, *cur=waitpid_head;
    for(;cur!=NULL;prev=cur,cur=cur->next){
      if(cur==p){
	/* found node */
	prev->next=cur->next;
	if(cur->next==NULL){
	  waitpid_tail=prev;
	}
	kfree(cur);
      }
    }
  }
  
}

void init_stdin_queue(){
	/* allocate 4KB of kernel memory for stdin buffer */
	termbuf=(char * )kmalloc(TERMBUF_SIZE);	
	termbuf_head=termbuf_tail=termbuf;
	isBufFull=0;
	read_kstack=(char * )kmalloc(FRAME_SIZE);	
}


int pipealloc(struct file ** f0,struct file ** f1){
	/* return 0 on success, -1 on failure  */
	struct pipe *p;
	p=0;
	*f0=*f1=0;
	if((*f0=filealloc())==0 || (*f1=filealloc())==0){
		/* printf("no free file struct available\n"); */
		/* free pipe structure */
		if(p)
			kfree(p);
		/* free file struct */
		if(*f0)
			fileclose(*f0);
		/* free file struct */
		if(*f1)
			fileclose(*f1);
		return -1;
	}
	p=(struct pipe *)kmalloc(sizeof(struct pipe));
	if(p==NULL){
		/* kmalloc failed */
		/* printf("kmalloc failed. no memory for pipe\n"); */
		/* free pipe structure */
		if(p)
			kfree(p);
		/* free file struct */
		if(*f0)
			fileclose(*f0);
		/* free file struct */
		if(*f1)
			fileclose(*f1);
		return -1;
	}
	p->readopen=1;
	p->writeopen=1;
	p->nread=0;
	p->nwrite=0;
	/* set up file struct for f0, which is read end of pipe */
	(*f0)->type=FD_PIPE;
	(*f0)->readable=1;
	(*f0)->writable=0;
	(*f0)->pipe=p;
	/* set up file struct for f1, which is write end of pipe */
	(*f1)->type=FD_PIPE;
	(*f1)->readable=0;
	(*f1)->writable=1;
	(*f1)->pipe=p;
	return 0;
}

struct file * filealloc(){
	/* return NULL on failure, file struct ptr on success */
	
	struct file *f;
	for(f=ftable.file;f < (ftable.file+NFILE);f++){
		if(f->ref==0){
			f->ref=1;
			return f;
		}
	}
	/* no avilable file structure */
	return NULL;
}

int fdalloc(struct file *f){
	/* return -1 on error, index of availabel local fd on succes which */
	/* traverse through process file entries to find a free entry */
	/* start from position 0 as 0,1,2 are already marked reserved for STDIN, STDOUT, STDERR */
	int fd;
	for(fd=0;fd<NOFILE;fd++){
		/* found a free space, put the file pointer at the same position */
		if(proc->ofile[fd]==0){
			proc->ofile[fd]=f;
			return fd;
		}
	}
	return -1;
}

void fileclose(struct file *f){
	
	struct file ff;
	/* if ref count of file less than one, panic */
	if(f->ref < 1){
		/* printf("reference count of file less than one..\n"); */
		/* panic. kill proc??? */
		return ;
	}

	/* if ref count is greater than one, decrement the count by one */
	else if(f->ref > 1){
		f->ref--;
		return ;
	}

	/* if ref count is equal to one then make it 0 and change type to FD_NONE */
	ff=*f;
	f->ref=0;
	f->type=FD_NONE;
	
	/* if file type is pipe */
	if(ff.type==FD_PIPE){
		pipeclose(ff.pipe, ff.writable);
	}
}

void pipeclose(struct pipe *p, int writable){
	/* pipe is write end, close the write end of the pipe */
	if(writable==1){
		p->writeopen=0;
		/* wakeup on nread */
		wakeup(&p->nread);
	}
	/* pipe is read end, close the read end of the pipe */
	else{
		p->readopen=0;
		/* wakeup on nwrite */
		wakeup(&p->nwrite);
	}
	
	/* if both ends are closed, free pipe */
	if(p->readopen==0 && p->writeopen==0){
		kfree(p);
	}
}

int pipewrite(struct pipe *p, char *addr, size_t n){
	/* return -1 on failure, number of bytes written on success */
	/* copy the n bytes from addr to pipe's data buffer */

	/* if not full, place addr into pipe's data buffer */
	int i;
	for(i=0;i<n;i++){
		/* pipe data buffer is full, wakeup the read end and sleep the write end  */
		while(p->nwrite == p->nread + PIPESIZE){
			/* pipe's read end is closed or process is killed/unused */
			if(p->readopen==0 || proc->state==UNUSED){
				return -1;
			}
			/* wakeup read end */
			wakeup(&p->nread);
			/* sleep write end */
			sleep(&p->nwrite);
		}
		
		p->data[p->nwrite++ % PIPESIZE]=addr[i];
	}
	/* data available for read end */
	/* wakeup read end */
	wakeup(&p->nread);
	return n;
}

int piperead(struct pipe *p, char *addr, size_t n){
	/* return -1 on failure, number of bytes read on success */
	/* buffer is empty, sleep the process */
	while(p->nread == p->nwrite && p->writeopen){
			if(proc->state==UNUSED){
				return -1;
			}
			/* sleep the read end */
			sleep(&p->nread);
	}
	
	/* copy the n bytes from pipe's data buffer to addr */
	int i;
	for(i=0;i<n;i++){
		/* pipe data buffer is empty, break and return i */
		if(p->nread == p->nwrite){
			break;
		}
		/* copy the contents of pipe buffer to addr */
		addr[i]=p->data[p->nread++ % PIPESIZE];
	}
	/* wakeup the write end of the pipe */
	wakeup(&p->nwrite);
	return i;
}


void sleep(void *chan){
	/* if(proc==1){ */
	/* 	panic("sleep"); */
	/* } */
	proc->chan =chan;
	proc->state=SLEEPING;
	sched();
	proc->chan=NULL;
	
}


void wakeup(void *chan){
	wakeup1(chan);
}

void wakeup1(void *chan){
	struct proc *p;
	for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
		if( (p->state==SLEEPING) && (p->chan == chan)){
			p->state=RUNNABLE;
		}
	}
}



struct inode *head=NULL;



int link_alloc(struct inode *p){
	/* returns first available position in link array */
	/* >=0 on success */
	/* -1 on failure */
	int i=0;
	for(i=0;i<NLINKS;i++){
		if(p->link[i]==NULL){
			/* empty */
			return i;
		}
	}
	return -1;
}


void init_inodes(){
	/* scan throught the tarfs */
	struct posix_header_ustar *p= (struct posix_header_ustar *)&_binary_tarfs_start;
	struct posix_header_ustar *p_e= (struct posix_header_ustar *)&_binary_tarfs_end;
	while(p<p_e && !(strlen(p->name)==0)){
		/* printf("cur name->%s, lookup name->%s, cur location->%p\n",p->name,name,p); */
		printf("cur name->%s, is of type->%c, size is %x, addr ->%p\n",p->name,*p->typeflag,round_up(oct_to_dec(p->size),512),p);
		/* add all directories */
		if(head==NULL){
		    /* create an entry for root */
		    head=(struct inode *)kmalloc(sizeof(struct inode));
			/* clear the contents of links */
			memset1((char *)head->link,0,sizeof(struct inode *)*NLINKS);
		    strcpy(head->name,"/");
		}
		int li;
		if((li=link_alloc(head)) < 0){
			printf("maximum number of directory entries(50) exceeded\n");
			for(;;)
				;
		}	   
		/* goto next file header by adding size of header and header->size */
		/* header->size should be rounded up next 512 multiple */
		//printf("%s->%u",oct_to_dec("00000 0000 0000"))
		p=(struct posix_header_ustar *)((char *)p+sizeof(struct posix_header_ustar ) + round_up(oct_to_dec(p->size),512));
	}
	
	
}

void do_ps(){
	struct proc *p;
	printf("PID    CMD\n");
	for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
		if(p->state!=UNUSED){
			printf("%d      %s\n",p->pid, p->name);
		}
	}
}

int do_kill(int pid){
	/* return 0 on success, -1 on failure */
	struct proc *p;
	for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
		if(p->pid==pid && p->state!=UNUSED){
			/* make the current process RUNNABLE */
			proc->state=RUNNABLE;
			/* exit the process */
			do_exit(0,p);
			return 0;
		}
	}
	return -1;
}

void set_wp_bit(){
	/* get cro register */
	uint64_t res=0;
	__asm__ __volatile__("movq %%cr0,%0;"
						 :"=r"(res)
						 );
	
	/* update cr0 register with 16th bit set */
	__asm__ __volatile__("movq %0,%%cr0;"
						 :
						 :"r"(res | (1ul<<16) )
						 );
	
}

void clear_wp_bit(){
	/* get cro register */
	uint64_t res=0;
	__asm__ __volatile__("movq %%cr0,%0;"
						 :"=r"(res)
						 );
	
	/* update cr0 register with 16th bit cleared */
	__asm__ __volatile__("movq %0,%%cr0;"
						 :
						 :"r"(res & ~(1ul<<16) )
						 );
	
}


void do_exit(int status, struct proc *p){

	/* printf("proc -> %d -> exit syscall\n",p->pid); */
	/* free vmas free_vma_list(head) */
	/* clear open file descriptors */
	int fd=0;
	for(fd=0;fd<NOFILE;fd++){
		if(p->ofile[fd]){	/* if local fd is present */
			fileclose(p->ofile[fd]); /* fileclose decrs refcount or marks file strct as unused */
			p->ofile[fd]=NULL;		/* delink */
		}
	}
	
	/* clear cwd  */
	memset1(p->cwd,0,NCHARS);	

	/* wakeup parent sleeping in waitpid */
	wakeup1(p->parent);

	/* if current process is present in stdin Q, and we decide to kill process */
	/* then dequeu proc from stdin Q first */
	/* this use case will mostly occur during page fault in do_copy */
	if(_stdin->proc == p){
		/* p is enqued in stdin Q */
		/* dequue from stdin Q */
		_stdin->proc=NULL;
	}
	

	/* pass abondoned children to initproc */
	struct proc *t;
	for(t=ptable.proc; t < &ptable.proc[NPROC];t++ ){
		if(t->parent == p){
			/* p is t's parent */
			t->parent=initproc;
			if(t->state == ZOMBIE){
				wakeup1(initproc);
			}
		}
	}
	p->state=ZOMBIE;
	/* call the sched */
	sched();
}


int do_waitpid(pid_t pid, int* status, int options){

	struct proc *p;
	int haveKids=0;
	if(pid==-1){
		
		/* if not specefic */
		/* look for any child */
		/* if it is zombie */
		/* mark it UNUSED */
		/* return pid */
		/* keep running till the end */
		/* if no zombie found, and a child exists in RUN, goto sleep */
		/* if no zombie and no child, return -1 */
		
		for(;;){
			/* return zombie if exists */
			haveKids=0;
			for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
				if(p->parent != proc){
					/* not a child of proc */
					continue;
				}
				haveKids=1;
				if(p->state == ZOMBIE){
					return free_res(p);
				}
			}
			/* has not found zombie */
			if(!haveKids){
				/* no kids, return -1, no point in sleeping */
				return -ECHILD;
			}
			/* some child exists which is running */
			/* sleep till that child wakes me */
			sleep(proc);
			
		}
		
	}
	else{
		/* if is specefic */
		/* then look for this pid */
		/* if zombie */
		/* mark it UNUSED */
		/* return pid */
		/* if running */
		/* goto sleep */
		/* later wake up and reap this one */
		/* if no child with this pid, then return -1 */
		for(;;){
			haveKids=0;
			for(p=ptable.proc;p<&ptable.proc[NPROC];p++){
				
				if(p->pid != pid){
					continue;
				}
				else{
					/* found process with id pid */
					/* check if this process is my child  */
					if(p->parent==proc){
						/* pid is my child */
						haveKids=1;
						/* check if it is zombie */
						/* then mark it UNUSED */
						if(p->state==ZOMBIE){
							return free_res(p);	/* returns pid */
						}
						/* else it is in use*/
						sleep(proc); /* sleep till child wakes u up */
						/* restart search, because we do not know which child woke me up */
						/* but we are only intersetd in process pid */
						/* p=ptable.proc; */
						/* p--; */
					}
					else{
						/* not my child */
						return -ECHILD;
					}
				}
			}
			if(haveKids==0){
				/* one complete scan done */
				/* pid proc does not exists */
				return -ECHILD;
			}

		}
		
	}
	/* printf("proc -> %d -> waitpid syscall\n",proc->pid); */

	/* /\* checks *\/ */
	/* /\* if pid = -1 *\/ */
	/* /\* check if there is any child of this process *\/ */

	/* /\* else  *\/ */
	/* /\* check if there is a child with this  *\/ */
	/* /\* change the process state to SLEEPING *\/ */
	/* proc->state=SLEEPING; */
	/* /\* add the process to a waitpid Q *\/ */
	/* if(enqueue_waitpid(proc,pid)==0){ */
	/* 	/\* enqueue failed *\/ */
	/* 	/\* waitpid returns -1 *\/ */
	/* 	proc->tf->rax=-1; */
	/* 	/\* make process running *\/ */
	/* 	proc->state=RUNNING; */
    /* return; */
	/* } */
	/* /\* enqueue success *\/ */
	/* /\* schedule another process *\/ */
	/* sched(); */
}


int free_res(struct proc *p){
	/* found a zombie */
	int ret=p->pid;
	kfree(p->kstack);
	p->kstack=NULL;
	free_vma_list(&(p->vma_head));
	/* free process page tables free_uvm(pml4_t) */
	free_uvm(p->pml4_t);
	p->state=UNUSED;
	p->pid=0;
	p->parent=NULL;
	p->name[0]=0;
	p->killed=0;
	p->size=0;
	p->isBg=0;
	/* return pid of zombie child */
	return ret;
	
}

void free_pcb(struct proc *p){
	/* free open files */
	int fd=0;
	for(fd=0;fd<NOFILE;fd++){
		if(p->ofile[fd]){/* if local fd is present */
			fileclose(p->ofile[fd]); /* fileclose decrs refcount or marks file strct as unused */
			p->ofile[fd]=NULL;/* delink */
		}
	}
	/* free the process kernel stack */
	kfree(p->kstack);	
	/* set proc state to UNUSED */
	p->state=UNUSED;
	p->kstack=NULL;
	p->pid=0;
	p->parent=NULL;
	p->name[0]=0;
	p->killed=0;
	
}
