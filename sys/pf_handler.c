#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 


void handle_pf(){
	printf("page fault handler\n");
	

}
