


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
		

# isr0 divide by zero exception
isr0:
		# cli		from mike's anser here piazza.com/class/i5glak0eumz4g9?cid=168
		# push is 8byte op, i.e stack pointer is decreased by 8bytes
		# first push is errono, which maybe dummy(0)
		# second push is interrupt number
		pushq 0
		pushq 0
		jmp isr_common

# isr1 debug exception
isr1:
		pushq 0
		pushq 1
		jmp isr_common
		
# isr2 debug exception
isr2:
		pushq 0
		pushq 2
		jmp isr_common


		
.global idt_load
		.extern idt_pointer
idt_load:
		# idt_pointer is a pointer to memory location which has req. info
		# load the value pointed to by idtpointer into LID register
		# () deferences the pointer to access contents
		lidt (idt_pointer)
		ret

.global isr_common
# from https://code.google.com/p/shovelos/source/browse/trunk/kernel/arch/x86_64/isr.c?r=182
isr_common:
		pushq %rax
		pushq %rcx
		pushq %rdx
		pushq %rsi
		pushq %rdi
		pushq %r8
		pushq %r9
		pushq %r10
		pushq %r11
		movq %rsp, %rdi
		call isr_handler
		popq %r11
		popq %r10
		popq %r9
		popq %r8
		popq %rdi
		popq %rsi
		popq %rdx
		popq %rcx
		popq %rax
		add 0x10,%rsp
		iretq

