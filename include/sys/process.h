#ifndef __PROCESS_H
#define __PROCESS_H

#include<sys/defs.h>
#include<sys/page.h>

#define KERNBASE 0xffffffff80000000ul

uint32_t proc_count;


typedef struct register_t{

uint64_t rax;
uint64_t rbx;
uint64_t rcx;
uint64_t rdx;
uint64_t rsi;
uint64_t rdi;
uint64_t r8;
uint64_t r9;
uint64_t r10;
uint64_t r11;
uint64_t r12;
uint64_t r13;
uint64_t r14;
uint64_t r15;

}reg_t;

typedef struct task{

uint64_t ppid; 				/* parent process id */
uint64_t pid;				/* process id */
uint64_t *stack;			/* user level stack virtual address */
reg_t regs;					/* saved GPR of the process */
uint64_t rip;				/* instruction pointer of user space*/
uint64_t rsp;				/* stack pointer of user stack*/
uint64_t cr3;				/* pml4 structure physical address */
pml4 *pml4_p;				/* pml4 structure virtual address */
void (* entry)();				/* user function entry point */
uint64_t state;				/* 0:ready, 1:wait, 2:sleep, 3:zombie */
uint64_t sleep_time;		/* sleep time */
struct task *next;

} pcb;


typedef struct run_queue{

	pcb *head;
	pcb *cur_pcb;				/* must implement this as a circular linked list */
}run_q;


run_q run_q_p;

uint64_t get_virt_addr(uint64_t x);

uint64_t get_phys_addr(uint64_t x);

void init_regs(reg_t *p);

int create_process();

void proc_entry();

#endif
