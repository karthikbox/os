#ifndef __SBUNIX_H
#define __SBUNIX_H

#include <sys/defs.h>

void printf(const char *fmt, ...);

/* declarations for console driver */
/* defined in /sys/printf.c */
void putchars(const char *str);
void putch(char c);
void move_cursor();
void scroll_down();
void clear_screen();
void set_color(int c);



// stack frame structure -> part of idt implementation
//this is how the stack frame looks lokme when isr_handler is being executed
struct stack_frame{
	

	uint64_t r11,r10,r9,r8,rdi,rsi,rdx,rcx,rax;//from isr_common
	uint64_t intr_num, error_num;//from isrX routine
	uint64_t rip,cs,rflags,rsp,ss;//from CPU during some process , happens automatically

};

#endif
