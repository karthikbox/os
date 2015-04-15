#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>

extern void trapret();
extern int exec(char *path,char **argv);
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
	p->tf->cs=0x8/* SEG_KCS() */;			/* user code segment */
	p->tf->ss=0x10/* SEG_KDS() */;			/* user data segment */
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
	char *argv[2];
	argv[0]=a;argv[1]=NULL;
	cli();
	exec("bin/hello",argv);
	//p->tf->cs=(SEG_);
	/* ltr(0x2Bu); */
	/* ltr 0x2B   with RPL of 3 for user??? */
	ltr(0x2Bu);

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
			printf("about to switch...bye\n");
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
	
	printf("tss.rsp0 -> %p\n",tss.rsp0);
	
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
