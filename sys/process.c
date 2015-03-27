#include<sys/sbunix.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>


uint32_t proc_count=1;			/* this is the pid counter. starts from 1. */

inline uint64_t get_virt_addr(uint64_t x){
	return KERNBASE+x;
}

inline uint64_t get_phys_addr(uint64_t x){
	return x-KERNBASE;
}

void init_regs(reg_t *p){
	p->rax=0;
	p->rbx=0;
	p->rcx=0;
	p->rdx=0;
	p->rsi=0;
	p->rdi=0;
	p->r8=0;
	p->r9=0;
	p->r10=0;
	p->r11=0;
	p->r12=0;
	p->r13=0;
	p->r14=0;
	p->r15=0;
}

void * kmalloc(size_t size){
	return NULL;
}

void init_runq(){
	run_q_p.head=NULL;
	run_q_p.cur_pcb=NULL;
}

int create_process(){
	/* return 0 if failure */
	/* return pid on success */
	
	/*  initialize PCB */
	
	/* kmalloc is a wrapper around alloc_frame.  */
	/* 	alloc_frame returns a physical address of 1 available page.  */
	/* 	kmalloc converts it into a virtual address and return it as void *.  */
	/* 	conversion into virtual address is done by adding kernbase to return value of alloc_frame */
	pcb *task=(pcb *)kmalloc(sizeof(pcb));

	/* create page tabel structures */
	pml4 *pml4_p=(pml4 *)kmalloc(sizeof(pml4)); /* returns virtual address */
	
	int i=0;
	for(i=0;i<ENTRIES_PER_PML4;i++){
		pml4_p->m_entries[i]=0x0ul;			/* clear pml4 entries */
	}
	pml4_p->m_entries[511]=*((uint64_t *)get_virt_addr((uint64_t)pml4_base)+511);	/* map kernel page tables into process page tables */
	task->pid=proc_count++;
	task->ppid=0;
	task->stack=(uint64_t *)kmalloc(FRAME_SIZE); /* this is the virt address of user stack. */
	/* this will be backed by a physical frame */
	/* when we load the ELF file, we will map the speciied stack segment start adress to this in the page tables*/
	init_regs(&(task->regs));	/* initialize GPR of the process */
	task->rip=0;
	task->rsp=0;				/* ??? */
	task->cr3=get_phys_addr((uint64_t)pml4_p);	
	task->pml4_p=pml4_p;
	task->entry=proc_entry;
	task->state=0;
	task->sleep_time=0;
	task->next=NULL;
	
	/* add it to run_q */
	

	return 0;
}


void proc_entry(){
	/* this is the entry point of the process. */
	/* this is executed by the kernel only in the first run of process*/
	/* this will load the binary */
	/* set up page tables */
}
