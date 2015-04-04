#ifndef __PROCESS_H
#define __PROCESS_H

#include<sys/defs.h>
#include<sys/page.h>

#define KERNBASE 0xffffffff80000000ul
#define NPROC 64				/* maximum number of processes */
#define KSTACKSIZE 4096			/* size of per-process kernel stack */
#define NOFILE 16				/* open files per process */
#define MAXARG 32				/* max exec arguments */

#define FL_IF 0x0000000000000200 /* interrupt enable */

#define GDT_CS        (0x00180000000000)  /*** code segment descriptor ***/
#define GDT_DS        (0x00100000000000)  /*** data segment descriptor ***/

#define C             (0x00040000000000)  /*** conforming ***/
#define DPL0          (0x00000000000000)  /*** descriptor privilege level 0 ***/
#define DPL1          (0x00200000000000)  /*** descriptor privilege level 1 ***/
#define DPL2          (0x00400000000000)  /*** descriptor privilege level 2 ***/
#define DPL3          (0x00600000000000)  /*** descriptor privilege level 3 ***/
#define P             (0x00800000000000)  /*** present ***/
#define L             (0x20000000000000)  /*** long mode ***/
#define D             (0x40000000000000)  /*** default op size ***/
#define W             (0x00020000000000)  /*** writable data segment ***/


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
 	return (uint64_t)(GDT_DS | P | W | DPL3);  /*** kernel data segment descriptor ***/
}


uint32_t proc_count;

/* process states */
enum procstate{

	UNUSED,
	EMBRYO,
	SLEEPING,
	RUNNABLE,
	RUNNING,
	ZOMBIE
};

struct context{
	uint64_t rbx;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	uint64_t rip;

};

struct trapframe{
/* trapframe generated by isr_common */
	uint64_t rbp;
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rax;

/* change in idt.c */
	/* uint64_t gs; */
	/* uint64_t fs; */
	/* uint64_t es; */
	/* uint64_t ds; */

	uint64_t intr_num;
	uint64_t error_code;

	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
};

/* PCB - process control block */
struct proc{
	uint64_t size;				/* process memory size in bytes */
	pml4 *pml4_t;				  /* pointer to pml4. */
	char *kstack;				  /* pointer to start of stack (i.e bottom of stack) */
	enum procstate state;		  /* process state */
	int pid;					  /* process identifiers */
	struct proc *parent;		  /* parent of process */
	struct trapframe *tf;		  /* stack trapframe */
	struct context *context;	  /* kernel stack */
	void *chan;					  /* if non-zero, sleeping on chan */
	int killed;					  /* if non-zero then killed */
	/* struct file *ofile[];		  /\* list of open files *\/ */
	char name[32];				  /* process name */
};

uint64_t get_virt_addr(uint64_t x);

uint64_t get_phys_addr(uint64_t x);

void userinit();
struct proc * alloc_proc();
void forkret();

pml4 *load_kern_vm();

#endif

/* typedef struct register_t{ */

/* uint64_t rax; */
/* uint64_t rbx; */
/* uint64_t rcx; */
/* uint64_t rdx; */
/* uint64_t rsi; */
/* uint64_t rdi; */
/* uint64_t r8; */
/* uint64_t r9; */
/* uint64_t r10; */
/* uint64_t r11; */
/* uint64_t r12; */
/* uint64_t r13; */
/* uint64_t r14; */
/* uint64_t r15; */
/* uint64_t rbp; */
/* }reg_t; */

/* typedef struct task{ */

/* 	uint64_t ppid; 				/\* parent process id *\/ */
/* 	uint64_t pid;				/\* process id *\/ */
/* 	uint64_t *stack;			/\* user level stack virtual address *\/ */
/* 	reg_t regs;					/\* saved GPR of the process *\/ */
/* 	uint64_t rsp;				/\* stack pointer of user stack*\/ */
/* 	uint64_t cr3; */
/* 	/\* uint64_t rip; *\/ */
/* 	pml4 *pml4_p;				/\* pml4 structure virtual address *\/ */
/* 	void (* entry)();				/\* user function entry point *\/ */
/* 	uint64_t state;				/\* 0:ready, 1:wait, 2:sleep, 3:zombie *\/ */
/* 	uint64_t sleep_time;		/\* sleep time *\/ */
/* 	struct task *next; */
	
/* } pcb; */


/* typedef struct run_queue{ */

/* 	pcb *head; */
/* 	pcb *tail; */
/* 	pcb *cur_pcb;				/\* must implement this as a circular linked list *\/ */
/* }run_q; */


/* run_q run_q_p; */

/* uint64_t get_virt_addr(uint64_t x); */

/* uint64_t get_phys_addr(uint64_t x); */

/* void init_regs(reg_t *p); */

/* int create_process(); */

/* void proc_entry(); */

/* void init_runq(); */

/* void switch_to_next(); */
/* #endif */
