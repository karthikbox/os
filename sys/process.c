#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>

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
		printf("inituvm: input sz more than a page\n");
	}
	mem=(char *)kmalloc(FRAME_SIZE);
	memset1(mem,0,FRAME_SIZE);
	
}

/* creates the first process */
void userinit(){
	struct proc *p;
	printf("entered userinit\n");
	memset1((char *)ptable.proc,0,sizeof(ptable));
	p=alloc_proc();
	initproc=p;
	if(!(p->pml4_t=load_kern_vm())){
		/* panic code goes here*/
		printf("unable to allocate pml4\n");
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
	
	
	printf("calling exec\n");
	/* call exec */
	proc=p;
	char *a="bin/hello";
	char *argv[3];
	argv[0]=a;argv[1]="os/sbunix";argv[2]=NULL;
	char *envp[7];
	envp[0]="PATH=/bin/ls:/bin/cat";
	envp[1]="cwd=/sbush/shell";
	envp[2]="envp1";
	envp[3]="envp2";
	envp[4]="envp3";
	envp[5]="envp4";
	envp[6]=NULL;
	cli();
	exec("bin/hello",argv,envp);
	//p->tf->cs=(SEG_);
	/* ltr(0x2Bu); */
	/* ltr 0x2B   with RPL of 3 for user??? */
	ltr(0x2Bu);

	/* initialize sleep_head and sleep_tail to NULL */
	init_sleep_queue();
	init_waitpid_queue();
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
			/* sp-=sizeof(struct context); */
			/* p->context=(struct context *)sp; */
			/* memset1((char *)p->context,0,sizeof(struct context)); */
			/* p->context->rip=(uint64_t)forkret; */
			return p;
			/* allocate page table and load ernel memory into it */
			
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
	
	p=proc+1;
	while(1){
		/* set interrupts ??? */
		for(;p<&ptable.proc[NPROC];p++){
			if(p->state!=RUNNABLE){
				continue;
			}
			/* switch to this process. */
			proc=p;
			switchuvm(p);
			p->state=RUNNING;
			//swtch(&cpu->scheduler,proc->context);
			__asm__ volatile(
					"movq %0,%%rsp;"
					:
					:"r"((char *)(proc->tf) - 8)
					:
							 );


			__asm__ volatile("retq;");
			
		}
		p=&ptable.proc[0];		/* restart from beginning */
	}

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
		printf("no page tables allocated for this process. They should have been. Fatal.\n");
		/* PANIC */
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

void free_pcb(struct proc *p){
	/* free the process kernel stack */
	kfree(p->kstack);
	/* set proc state to UNUSED */
	p->state=UNUSED;
}


int enqueue_sleep(struct proc *p,struct timespec *rem){
	/* returns 0 on failure and 1 on success */
	struct sleep_entry *t=(struct sleep_entry *)kmalloc(sizeof(struct sleep_entry));
	t->proc=p;
	t->rem.tv_sec=rem->tv_sec;
	t->rem.tv_nsec=rem->tv_nsec;
	t->next=NULL;
	if(t==NULL){
		printf("unable to allocate memory...enqueue_sleep failed\n");
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
    printf("unable to allocate memory..enqueue_waitpid failed\n");
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

