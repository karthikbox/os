#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/syscall.h>


int brk(void *end_data_segment){
    void * ret = (void *) syscall_1(SYS_brk,(uint64_t)end_data_segment);
    if((uint64_t)ret==(uint64_t)end_data_segment){
	printf("success1\n");
	return 0;
    }
    else{
	return -1;
    }    
}

void * sbrk(void *offset){
    //assuming NULL is invalid address, sys_brk return the current brk pointer
    //sbrk will return cur_brk, whatever the offset
    // heap brk decreases if offset is negative
    // sbrk or brk will not decrease brk below the default brk set by kernel
    //when program is loaded
    void * cur_brk = (void *) syscall_1(SYS_brk,(uint64_t)NULL);
    void * new_brk=(void *)((uint64_t)cur_brk+(uint64_t)offset);
    if(brk(new_brk)==-1){
	//brk() did not set brk pointer to new_brk
	printf("success2\n");
	return (void *)-1;
    }
    else{
	//brk() set the brk pointer to new_brk
	printf("success3\n");
	return cur_brk;
    }
    
}

void *malloc(size_t size){
    return NULL;
}
