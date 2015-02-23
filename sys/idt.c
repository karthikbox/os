#include<sys/sbunix.h>
#include <sys/utility.h>

/* definition of idt entry */
struct idt_entry{
	uint16_t offset_0_15;
    uint16_t selector;
    unsigned ist : 3 ;
    unsigned reserved0 : 5;
    unsigned type : 4;
    unsigned zero : 1;
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

extern void idt_load();


/* sets a particular the idt entry */
void idt_entry_set(){


}




/* install the idt entry */
void idt_init(){
	
	idt_pointer.limit=sizeof(idt_table)-1; //address of the end of the table
	idt_pointer.base=(uint64_t *)idt_table;//address of start of tabe

	//clear out idt memory
	memset1((char *)idt_table,0,sizeof(idt_table));
	//load idt_pointer to the cpu interrupt table register
	idt_load();
	
	

}

void isr_handler(struct stack_frame *s){
	if(s->intr_num <= 31){
		//exception
		
	}

}
