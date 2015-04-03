


.text

# isr0-31 are for exceptions
.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7
.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15
.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23
.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31
.global isr32
.global isr33
.global isr128
		

# isr0 divide by zero exception
isr0:
		# cli		from mike's anser here piazza.com/class/i5glak0eumz4g9?cid=168
		# push is 8byte op, i.e stack pointer is decreased by 8bytes
		# first push is errono, which maybe dummy(0)
		# second push is interrupt number
		pushq $0
		pushq $0
		jmp isr_common

# isr1 debug exception
isr1:
		pushq $0
		pushq $1
		jmp isr_common
		
# isr2 debug exception
isr2:
		pushq $0
		pushq $2
		jmp isr_common

# Int 3 Exception
isr3:
		    pushq $0
		    pushq $3
		    jmp isr_common

# INTO Exception
isr4:
		    pushq $0
		    pushq $4
		    jmp isr_common

#   5: Out of Bounds Exception
isr5:
		    
		    pushq $0
		    pushq $5
		    jmp isr_common

#   6: Invalid Opcode Exception
isr6:
		    
		    pushq $0
		    pushq $6
		    jmp isr_common

#   7: Coprocessor Not Available Exception
isr7:
		    
		    pushq $0
		    pushq $7
		    jmp isr_common

#   8: Double Fault Exception (With Error Code!)
isr8:
		    
		    pushq $8
		    jmp isr_common

#   9: Coprocessor Segment Overrun Exception
isr9:
		    
		    pushq $0
		    pushq $9
		    jmp isr_common

#  10: Bad TSS Exception (With Error Code!)
isr10:
		    
		    pushq $10
		    jmp isr_common

#  11: Segment Not Present Exception (With Error Code!)
isr11:
		    
		    pushq $11
		    jmp isr_common

#  12: Stack Fault Exception (With Error Code!)
isr12:
		    
		    pushq $12
		    jmp isr_common

#  13: General Protection Fault Exception (With Error Code!)
isr13:
		    
		    pushq $13
		    jmp isr_common

#  14: Page Fault Exception (With Error Code!)
isr14:
		    
		    pushq $14
		    jmp isr_common

#  15: Reserved Exception
isr15:
		    
		    pushq $0
		    pushq $15
		    jmp isr_common

#  16: Floating Point Exception
isr16:
		    
		    pushq $0
		    pushq $16
		    jmp isr_common

#  17: Alignment Check Exception
isr17:
		    
		    pushq $0
		    pushq $17
		    jmp isr_common

#  18: Machine Check Exception
isr18:
		    
		    pushq $0
		    pushq $18
		    jmp isr_common

#  19: Reserved
isr19:
		    
		    pushq $0
		    pushq $19
		    jmp isr_common

#  20: Reserved
isr20:
		    
		    pushq $0
		    pushq $20
		    jmp isr_common

#  21: Reserved
isr21:
		    
		    pushq $0
		    pushq $21
		    jmp isr_common

#  22: Reserved
isr22:
		    
		    pushq $0
		    pushq $22
		    jmp isr_common

#  23: Reserved
isr23:
		    
		    pushq $0
		    pushq $23
		    jmp isr_common

#  24: Reserved
isr24:
		    
		    pushq $0
		    pushq $24
		    jmp isr_common

#  25: Reserved
isr25:
		    
		    pushq $0
		    pushq $25
		    jmp isr_common

#  26: Reserved
isr26:
		    
		    pushq $0
		    pushq $26
		    jmp isr_common

#  27: Reserved
isr27:
		    
		    pushq $0
		    pushq $27
		    jmp isr_common

#  28: Reserved
isr28:
		    
		    pushq $0
		    pushq $28
		    jmp isr_common

#  29: Reserved
isr29:
		    
		    pushq $0
		    pushq $29
		    jmp isr_common

#  30: Reserved
isr30:
		    
		    pushq $0
		    pushq $30
		    jmp isr_common

#  31: Reserved
isr31:
		    
		    pushq $0
		    pushq $31
		    jmp isr_common

#  32: Timer Interrupt
isr32:
		    
		    pushq $0
		    pushq $32
		    jmp isr_common

#  33: Key Board Interrupt
isr33:
		    
		    pushq $0
		    pushq $33
		    jmp isr_common

#  128: int 0x80
isr128:
			pushq $0
		    pushq $128
		    jmp isr_common

		

		
.global idt_load
		.extern idt_pointer
idt_load:
		# idt_pointer is a pointer to memory location which has req. info
		# load the value pointed to by idtpointer into LID register
		# () deferences the pointer to access contents
		lidt (idt_pointer)
		retq

.global isr_common
# from https://code.google.com/p/shovelos/source/browse/trunk/kernel/arch/x86_64/isr.c?r=182
		.extern isr_handler
isr_common:
#change in process.h and idt.c		
		pushq %rax
		pushq %rbx		
		pushq %rcx
		pushq %rdx
		pushq %rsi
		pushq %rdi
		pushq %r8
		pushq %r9
		pushq %r10
		pushq %r11
		pushq %r12
		pushq %r13
		pushq %r14
		pushq %r15
		pushq %rbp
		movq %rsp, %rdi
		call isr_handler
.global trapret
trapret:		
		popq %rbp
		popq %r15
		popq %r14
		popq %r13
		popq %r12
		popq %r11
		popq %r10
		popq %r9
		popq %r8
		popq %rdi
		popq %rsi
		popq %rdx
		popq %rcx
		popq %rbx
		popq %rax
		addq $0x10,%rsp
		iretq

