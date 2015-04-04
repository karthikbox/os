#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>

extern void trapret();
extern int exec(char *path,char **argv);
static struct proc *initproc;

uint32_t proc_count=1;			/* this is the pid counter. starts from 1. */

inline uint64_t get_virt_addr(uint64_t x){
	return KERNBASE+x;
}

inline uint64_t get_phys_addr(uint64_t x){
	return x-KERNBASE;
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
	pml4_t[511]=pml4_base[511];	/* put kernel space which is marked RW for only supervisor into process pml4 table */
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
	p->tf->cs=SEG_UCS();			/* user code segment */
	p->tf->ss=SEG_UDS();			/* user data segment */
	p->tf->rflags=FL_IF;			/* enable interrupts */
	p->tf->rsp=FRAME_SIZE; /* where does the user stack star from in the proc vm */
	p->tf->rip=0;		   /* begining of initcode.S */
	strcpy(p->name,"initcode");	/* p->name has a size of 32 bytes */
	p->state=RUNNABLE;
	/* call exec */
	exec("bin/hello",NULL);
	//p->tf->cs=(SEG_);
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
			sp-=sizeof(struct context);
			p->context=(struct context *)sp;
			memset1((char *)p->context,0,sizeof(struct context));
			p->context->rip=(uint64_t)forkret;
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



//uncomment till here



/* void init_regs(reg_t *p){ */
/* 	p->rax=0; */
/* 	p->rbx=0; */
/* 	p->rcx=0; */
/* 	p->rdx=0; */
/* 	p->rsi=0; */
/* 	p->rdi=0; */
/* 	p->r8=0; */
/* 	p->r9=0; */
/* 	p->r10=0; */
/* 	p->r11=0; */
/* 	p->r12=0; */
/* 	p->r13=0; */
/* 	p->r14=0; */
/* 	p->r15=0; */
/* } */

/* void * kmalloc(size_t size){ */
/* 	return NULL; */
/* } */

/* void init_runq(){ */
/* 	run_q_p.head=NULL; */
/* 	run_q_p.tail=NULL; */
/* 	run_q_p.cur_pcb=NULL; */
/* } */

/* void insert_head_runq(pcb *t){ */
/* 	if(!run_q_p.head){ */
/* 		/\* Q is empty *\/ */
/* 		run_q_p.head=t; */
/* 		run_q_p.tail=t; */
/* 	} */
/* 	else{ */
/* 		t->next=run_q_p.head; */
/* 		run_q_p.head=t; */
/* 	} */
/* } */

/* void insert_tail_runq(pcb *t){ */
/* 	if(!run_q_p.head){ */
/* 		/\* Q is empty *\/ */
/* 		run_q_p.head=t; */
/* 		run_q_p.tail=t; */
/* 	} */
/* 	else{ */
/* 		run_q_p.tail->next=t; */
/* 		run_q_p.tail=t; */
/* 	} */
/* } */

/* int create_process(){ */
/* 	/\* return 0 if failure *\/ */
/* 	/\* return pid on success *\/ */
	
/* 	/\*  initialize PCB *\/ */
	
/* 	/\* kmalloc is a wrapper around alloc_frame.  *\/ */
/* 	/\* 	alloc_frame returns a physical address of 1 available page.  *\/ */
/* 	/\* 	kmalloc converts it into a virtual address and return it as void *.  *\/ */
/* 	/\* 	conversion into virtual address is done by adding kernbase to return value of alloc_frame *\/ */
/* 	pcb *task=(pcb *)kmalloc(sizeof(pcb)); */

/* 	/\* create page tabel structures *\/ */
/* 	pml4 *pml4_p=(pml4 *)kmalloc(sizeof(pml4)); /\* returns virtual address *\/ */
	
/* 	int i=0; */
/* 	for(i=0;i<ENTRIES_PER_PML4;i++){ */
/* 		pml4_p->m_entries[i]=0x0ul;			/\* clear pml4 entries *\/ */
/* 	} */
/* 	pml4_p->m_entries[511]=*((uint64_t *)get_virt_addr((uint64_t)pml4_base)+511);	/\* map kernel page tables into process page tables *\/ */
/* 	task->pid=proc_count++; */
/* 	task->ppid=0; */
/* 	task->stack=(uint64_t *)kmalloc(FRAME_SIZE); /\* this is the virt address of user stack. *\/ */
/* 	/\* this will be backed by a physical frame *\/ */
/* 	/\* when we load the ELF file, we will map the speciied stack segment start adress to this in the page tables*\/ */
/* 	init_regs(&(task->regs));	/\* initialize GPR of the process *\/ */
/* 	task->regs.rip=0; */
/* 	task->regs.rsp=0;				/\* ??? *\/ */
/* 	task->regs.cr3=get_phys_addr((uint64_t)pml4_p);	 */
/* 	task->pml4_p=pml4_p; */
/* 	task->entry=proc_entry; */
/* 	task->state=0; */
/* 	task->sleep_time=0; */
/* 	task->next=NULL; */
	
/* 	/\* add it to run_q *\/ */
/* 	insert_tail_runq(task); */
/* 	return 0; */
/* } */


/* void proc_entry(){ */
/* 	/\* this is the entry point of the process. *\/ */
/* 	/\* this is executed by the kernel only in the first run of process*\/ */
/* 	/\* this will load the binary *\/ */
/* 	/\* set up page tables *\/ */
/* } */

/* void schedule(){ */
/* 	/\* __asm volatile( *\/ */
/* 	/\* 			   "pushq %rax\n\t" *\/ */
/* 	/\* 			   "pushq %rbx\n\t" *\/ */
/* 	/\* 			   "pushq %rcx\n\t" *\/ */
/* 	/\* 			   "pushq %rdx\n\t" *\/ */
/* 	/\* 			   "pushq %rsi\n\t" *\/ */
/* 	/\* 			   "pushq %rdi\n\t" *\/ */
/* 	/\* 			   "pushq %r8\n\t" *\/ */
/* 	/\* 			   "pushq %r9\n\t" *\/ */
/* 	/\* 			   "pushq %r10\n\t" *\/ */
/* 	/\* 			   "pushq %r11\n\t" *\/ */
/* 	/\* 			   "pushq %r12\n\t" *\/ */
/* 	/\* 			   "pushq %r13\n\t" *\/ */
/* 	/\* 			   "pushq %r14\n\t" *\/ */
/* 	/\* 			   "pushq %r15\n\t" *\/ */
/* 	/\* 			   "pushq %rsp\n\t" *\/ */
/* 	/\* 			   ) *\/ */
/* 	reg_t *stack_frame; */
/* 	__asm volatile( */
/* 				   "pushq %%rax;" */
/* 				   "pushq %%rbx;" */
/* 				   "pushq %%rcx;" */
/* 				   "pushq %%rdx;" */
/* 				   "pushq %%rsi;" */
/* 				   "pushq %%rdi;" */
/* 				   "pushq %%r8;" */
/* 				   "pushq %%r9;" */
/* 				   "pushq %%r10;" */
/* 				   "pushq %%r11;" */
/* 				   "pushq %%r12;" */
/* 				   "pushq %%r13;" */
/* 				   "pushq %%r14;" */
/* 				   "pushq %%r15;" */
/* 				   "pushq %%rbp;" */
/* 				   "movq %%rsp,%0;" */
/* 				   "movq %%cr3,%1;" */
/* 				   :"=r"(stack_frame),"=r"(run_q_p.cur_pcb->cr3)  */
/* 				   : */
/* 				   ); */
/* 	//memcpy(void) */
/* 	//save_to(stack_frame,run_q_p.cur_pcb); */
/* 	/\* save all registers to pcb *\/ */
/* 	run_q_p.cur_pcb->rsp=(uint64_t)stack_frame; */
/* 	memcpy(&(run_q_p.cur_pcb->regs),stack_frame,sizeof(reg_t)); */
/* 	/\* switch to next task *\/ */
/* 	switch_to_next(); */
/* 	/\* restore stack frame from the PCB *\/ */
/* 	memcpy(run_q_p.cur_pcb->rsp,&(run_q_p.cur_pcb->regs),sizeof(reg_t)); */
	
/* 	/\* pop stack frame into respective registers *\/ */
/* 	__asm volatile( */
/* 				   "movq %0,%%rsp;" */
/* 				   "popq %%rbp;" */
/* 				   "popq %%r15;" */
/* 				   "popq %%r14;" */
/* 				   "popq %%r13;" */
/* 				   "popq %%r12;" */
/* 				   "popq %%r11;" */
/* 				   "popq %%r10;" */
/* 				   "popq %%r9;" */
/* 				   "popq %%r8;" */
/* 				   "popq %%rdi;" */
/* 				   "popq %%rsi;" */
/* 				   "popq %%rdx;" */
/* 				   "popq %%rcx;" */
/* 				   "popq %%rbx;" */
/* 				   "popq %%rax;" */
/* 				   : */
/* 				   :(run_q_p.cur_pcb->rsp) */
/* 				   );	 */
/* 	/\* load cr3 *\/ */
/* 	load_base(run_q_p.cur_pcb->cr3); */
	
/* } */

/* void switch_to_next(){ */

/* } */
/*  */
