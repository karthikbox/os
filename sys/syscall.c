#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 

void yield(){
	printf("yield syscall\n");
	proc->state=RUNNABLE;
	scheduler();
	
}
