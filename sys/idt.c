#include <sys/sbunix.h>
#include <sys/utility.h>
#include<sys/syscall.h>
#include<sys/process.h>
#include<sys/memory.h>
/* definition of idt entry */
int exec(char *filename,char **kargv,char **kenvp);
struct idt_entry{
	uint16_t offset_0_15;
    uint16_t selector;
    unsigned ist : 3 ;
    unsigned reserved0 : 5;
    unsigned type : 4;
    unsigned strg_seg : 1;
    unsigned dpl : 2;
    unsigned p : 1;
    uint16_t offset_31_16;
    uint32_t offset_63_32;
    uint32_t reserved1;

}__attribute__((packed));


/* lidt man entry at pageend of http://wiki.osdev.org/Interrupt_Descriptor_Table */
/* total 10bytes. */
/* lower 2 bytes for limit */
/* higher 4 bytes for base */
struct idt_ptr{
	uint16_t limit;
	uint64_t *base;
}__attribute__((packed));

struct idt_entry idt_table[256]; /*this is the IDT table */
struct idt_ptr idt_pointer; /* use this to set IDTS register of cpu */


// stack frame structure -> part of idt implementation
//this is how the stack frame looks lokme when isr_handler is being executed
struct stack_frame{
	/* uint64_t r11,r10,r9,r8,rdi,rsi,rdx,rcx,rax;//from isr_common */
	/* uint64_t intr_num, error_num;//from isrX routine */
	/* uint64_t rip,cs,rflags,rsp,ss;//from CPU during some process , happens automatically */
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
	
	/* change in process.h */
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


extern void idt_load();
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();
extern void isr32();
extern void isr33();
extern void isr128();

/* sets a particular the idt entry */
void idt_entry_set(int n,uint16_t selector,char ist,char gate_type,char dpl,char present,uint64_t target){
	struct idt_entry *entry=&idt_table[n];
	entry->offset_0_15= target & 0xffff;//offset low 0-15
	entry->selector=selector;
	entry->ist=ist;//mostlikely 0
	entry->reserved0=0;//0 by default
	entry->type=gate_type;//gate_type=0xE if interrupt, else 0xF for traps
	entry->strg_seg=0;//storage segment 0 by default
	entry->dpl=dpl;// descriptor previledge level, 0 by default for interrupt and traps
	entry->p=present;// present =1 if IRQ handlre is present
	entry->offset_31_16= (target>>16) & 0xffff;//offset 16-31
	entry->offset_63_32=(target>>32) & 0xffffffff;//offset 63-32
	entry->reserved1=0;//default

}

char *exception_description[]={
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Timer Interrupt",
    "Keyboard Interrupt"
};

void pic_init(){
	//remaps the PICS; IRQs 0-7 are mapped to entries 8-15 
	//8-15 entries are actually for software exceptions 
	//so we need to remap those entries 32-57 in the idt table
	//TODO: Implement outportb function.
	outportb(0x20, 0x11);
    outportb(0xA0, 0x11);
    outportb(0x21, 0x20);
    outportb(0xA1, 0x28);
    outportb(0x21, 0x04);
    outportb(0xA1, 0x02);
    outportb(0x21, 0x01);
    outportb(0xA1, 0x01);
    outportb(0x21, 0x0);
    outportb(0xA1, 0x0);
}



/* install the idt entry */
void idt_init(){
	
	idt_pointer.limit=sizeof(idt_table)-1; //address of the end of the table
	idt_pointer.base=(uint64_t *)idt_table;//address of start of tabe

	pic_init();
	//clear out idt memory
	memset1((char *)idt_table,0,sizeof(idt_table));
	printf("%d\n", sizeof(idt_table));

	//do idt_entry_set for every interrupt
	// void idt_entry_set(int n,uint16_t selector,char ist,char gate_type,char dpl,char present,uint64_t *target)
	idt_entry_set(0,0x8,0,0xE,0,1,(uint64_t)&isr0);//0xE for interrupts
	idt_entry_set(1,0x8,0,0xE,0,1,(uint64_t)&isr1);//0xE for interrupts
	idt_entry_set(2,0x8,0,0xE,0,1,(uint64_t)&isr2);//0xE for interrupts
	idt_entry_set(3,0x8,0,0xE,0,1,(uint64_t)&isr3);//0xE for interrupts
	idt_entry_set(4,0x8,0,0xE,0,1,(uint64_t)&isr4);//0xE for interrupts
	idt_entry_set(5,0x8,0,0xE,0,1,(uint64_t)&isr5);//0xE for interrupts
	idt_entry_set(6,0x8,0,0xE,0,1,(uint64_t)&isr6);//0xE for interrupts
	idt_entry_set(7,0x8,0,0xE,0,1,(uint64_t)&isr7);//0xE for interrupts
	idt_entry_set(8,0x8,0,0xE,0,1,(uint64_t)&isr8);//0xE for interrupts
	idt_entry_set(9,0x8,0,0xE,0,1,(uint64_t)&isr9);//0xE for interrupts
	idt_entry_set(10,0x8,0,0xE,0,1,(uint64_t)&isr10);//0xE for interrupts
	idt_entry_set(11,0x8,0,0xE,0,1,(uint64_t)&isr11);//0xE for interrupts
	idt_entry_set(12,0x8,0,0xE,0,1,(uint64_t)&isr12);//0xE for interrupts
	idt_entry_set(13,0x8,0,0xE,0,1,(uint64_t)&isr13);//0xE for interrupts
	idt_entry_set(14,0x8,0,0xE,0,1,(uint64_t)&isr14);//0xE for interrupts
	idt_entry_set(15,0x8,0,0xE,0,1,(uint64_t)&isr15);//0xE for interrupts
	idt_entry_set(16,0x8,0,0xE,0,1,(uint64_t)&isr16);//0xE for interrupts
	idt_entry_set(17,0x8,0,0xE,0,1,(uint64_t)&isr17);//0xE for interrupts
	idt_entry_set(18,0x8,0,0xE,0,1,(uint64_t)&isr18);//0xE for interrupts
	idt_entry_set(19,0x8,0,0xE,0,1,(uint64_t)&isr19);//0xE for interrupts
	idt_entry_set(20,0x8,0,0xE,0,1,(uint64_t)&isr20);//0xE for interrupts
	idt_entry_set(21,0x8,0,0xE,0,1,(uint64_t)&isr21);//0xE for interrupts
	idt_entry_set(22,0x8,0,0xE,0,1,(uint64_t)&isr22);//0xE for interrupts
	idt_entry_set(23,0x8,0,0xE,0,1,(uint64_t)&isr23);//0xE for interrupts
	idt_entry_set(24,0x8,0,0xE,0,1,(uint64_t)&isr24);//0xE for interrupts
	idt_entry_set(25,0x8,0,0xE,0,1,(uint64_t)&isr25);//0xE for interrupts
	idt_entry_set(26,0x8,0,0xE,0,1,(uint64_t)&isr26);//0xE for interrupts
	idt_entry_set(27,0x8,0,0xE,0,1,(uint64_t)&isr27);//0xE for interrupts
	idt_entry_set(28,0x8,0,0xE,0,1,(uint64_t)&isr28);//0xE for interrupts
	idt_entry_set(29,0x8,0,0xE,0,1,(uint64_t)&isr29);//0xE for interrupts
	idt_entry_set(30,0x8,0,0xE,0,1,(uint64_t)&isr30);//0xE for interrupts
	idt_entry_set(31,0x8,0,0xE,0,1,(uint64_t)&isr31);//0xE for interrupts
	idt_entry_set(32,0x8,0,0xE,0,1,(uint64_t)&isr32);//0xE for interrupts
	idt_entry_set(33,0x8,0,0xE,0,1,(uint64_t)&isr33);//0xE for interrupts
	idt_entry_set(0x80,0x8,0,0xE,3,1,(uint64_t)&isr128); /* set DPL as 3 */

	//load idt_pointer to the cpu interrupt table register
	idt_load();
	__asm__("sti");
}

void isr_handler(struct stack_frame *s){
	if(s->intr_num == 14){
		/* page failt handler */
		handle_pf(s->error_code);
	}
	else if(s->intr_num <= 31){
		//exception
		printf("proc id -> %d got %s\n",proc->pid,exception_description[s->intr_num]);
		printf("Execution halted. Kernel entering infinite loop\n");
		for(;;);
	}
	else if (s->intr_num == 32)
	{
		//Timer Interrupt
		//printf("%s\n",exception_description[s->intr_num]);
		//printf("Execution halted. Kernel entering infinite loop\n");
		//printf("proc id -> %d -> timer int\n",proc->pid);
		outportb(0x20, 0x20);
		timer_handler();
		//for(;;);
	}
	else if (s->intr_num == 33)
	{
		//Keyboard Interrupt
		//printf("%s\n",exception_description[s->intr_num]);
		//printf("Execution halted. Kernel entering infinite loop\n");
		//printf("proc id -> %d -> keyboard int\n",proc->pid);
		outportb(0x20, 0x20);
		keyboard_handler();
		//for(;;);
	}
	else if(s->intr_num == T_SYSCALL){
		if(s->rax==SYS_yield){
			do_yield();
		}
		else if(s->rax==SYS_fork){
			do_fork();
		}
		else if(s->rax==SYS_write){
			s->rax=do_write((int)s->rdi,(const void *)s->rsi,(size_t)s->rdx);
		}
		else if(s->rax==SYS_brk){
			do_brk((void *)s->rdi);
		}
		else if(s->rax==SYS_exit){
			do_exit((int)s->rdi,proc);
		}
		else if(s->rax==SYS_getpid){
			s->rax=do_getpid();
		}
		else if(s->rax==SYS_getppid){
			s->rax=do_getppid();
		}
		else if(s->rax==SYS_nanosleep){
			do_nanosleep((struct timespec *)s->rdi,(struct timespec *)s->rsi);
		}
		else if(s->rax==SYS_wait4){
			do_waitpid((pid_t)s->rdi, (int*)s->rsi, (int)s->rdx);
		}
		else if(s->rax==SYS_execve){
			/* copy the contents of argv and envp to kernel stack */
			char *filename=(char *)s->rdi;
			char **argv=(char **)s->rsi;
			char **envp=(char **)s->rdx;
			int argc=0;
			int envc=0;

			while(argv[argc]){
				argc++;
			}
			argc++;
			while(envp[envc]){
				envc++;
			}
			envc++;

			char *kfilename=(char *)kmalloc(sizeof(char)*(strlen(filename)+1));
			strcpy(kfilename,filename);

			char **kargv=(char **)kmalloc(argc*sizeof(char *));
			char **kenvp=(char **)kmalloc(envc*sizeof(char*));
			/* clear kargv and kenvp */
			memset1((char *)kargv,0,8*argc);
			memset1((char *)kenvp,0,8*envc);
			/* copy every argv into kargv */
			int i;
			for(i=0;argv[i];i++){
				/* allocate memory */
				char *mem=(char *)kmalloc(sizeof(char)*(strlen(argv[i])+1));
				/* copy argv into mem */
				strcpy(mem,argv[i]);
				/* put mem into kargv */
				kargv[i]=mem;
			}
			/* copy envp into kenvp */
			for(i=0;envp[i];i++){
				/* allocate memory */
				char *mem=(char *)kmalloc(sizeof(char)*(strlen(envp[i])+1));
				/* copy envp into mem */
				strcpy(mem,envp[i]);
				/* put mem into kargv */
				kenvp[i]=mem;
			}
			int ret=exec(kfilename,kargv,kenvp);
			/* free allocated kargv and kenvp since exec makes a copy of these in user stack */
			/* free kargv */
			for(i=0;kargv[i];i++){
				kfree(kargv[i]);
			}
			kfree(kargv);
			/* free kenvp */
			for(i=0;kenvp[i];i++){
				kfree(kenvp[i]);
			}
			kfree(kenvp);
			/* free filename */
			kfree(kfilename);
			/* put return value of exec in rax and return to user */
			s->rax=ret;

		}
		else if(s->rax==SYS_read){

			do_read((int)s->rdi, (void *)s->rsi, (size_t)s->rdx);
		}
		else if(s->rax==SYS_pipe){
			do_pipe((int *)s->rdi);
		}
		else if(s->rax==SYS_close){
			do_close((int)s->rdi);
		}
		else if(s->rax==SYS_dup){
			s->rax=do_dup((int)s->rdi);
		}
		else if(s->rax==SYS_dup2){
			s->rax=do_dup2((int)s->rdi,(int)s->rsi);
		}
		else if(s->rax==SYS_getcwd){
			s->rax=(uint64_t)do_getcwd((char *)s->rdi,(size_t)s->rsi);
		}
		else if(s->rax==SYS_chdir){
			s->rax=do_chdir((const char*) s->rdi);
		}
		else if(s->rax==SYS_open){
			s->rax=do_open((char *)s->rdi, (uint64_t)s->rsi);
		}
		else if(s->rax==SYS_getdents){
			s->rax=do_getdents((int)s->rdi, (char *)s->rsi, (size_t)s->rdx);
		}
		else{
			printf("incorrect syscall number\nproc pid->%d\n",proc->pid);
		}
	}
}
