#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 

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


void yield(){
	printf("yield syscall\n");
	proc->state=RUNNABLE;
	scheduler();
	
}

void do_fork(){
	printf("fork syscall\n");
	/* alloc_proc finds a spot in pcb array and allocates kstack and sets sp and tf pointers and gives a pid*/
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

	/* copy VMAs from parent to child */
	if((p->vma_head=copyvma(proc->vma_head))==NULL){
		/* copyvma failed */
		printf("unable to fork. no space for new processes vmas\n");
		proc->tf->rax=-1;
		/* copyvma takes care of freeing new procs vmas */
		/* if any failure in copyuvm or copyvma, then deallocate p's resources */
		/* such as kernel stack and set pcb of p to UNUSED and free_uvm(p->pml4)*/
		/* free_uvm(p->pml4_t) */
		free_pcb(p);
		return ;
	}
	

	/* copy trapframe of parent to child */
	memcpy(p->tf,proc->tf,sizeof(struct trapframe));
	/* set return of fork of child to 0 */
	p->tf->rax=0;
	/* set size of child same as parent */
	p->size=proc->size;
	/* set parent of child to proc */
	p->parent=proc;
	/* give child the same name as parent */
	strcpy(p->name,proc->name);
	/* return pid of child to parent */
	proc->tf->rax=p->pid;
	/* set state of child tyo runnable */
	p->state=RUNNABLE;
	
	
}
