# Initial process execs /init.
#include "syscall.h"
#include "traps.h"
# exec(init, argv)
.globl start
start:
		pushq $argv
		pushq $init
		pushq $0 // where caller pc would be
		movq $SYS_exec, %eax
		int $T_SYSCALL
#infinite loop, when exec fails
exit:
		movq $SYS_exit, %eax
		int $T_SYSCALL
		jmp exit

# char init[] = "/init\0" ;
init:
		.string "/init\0"

# char *argv[] = { init, 0 } ;
.p2align 3
argv:
		.long init
		.long 0
