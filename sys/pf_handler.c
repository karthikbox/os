#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/syscall.h> 

enum PF_ERROR_CODE_FLAGS{
	
	PF_PRESENT = 1ul,			/* 0-> non present page, 1->page present,but protection violation */
	PF_WRITE   = 2ul,			/* 0->read violation, 1->write violation */
	PF_USER    = 4ul,			/* 0-> PF caused during supervisor mode, 1->during user mode  */
	PF_RSVD    = 8ul,			/* no needed */
	PF_IFETCH  = 0x10ul			/* not needed */

};


uint64_t getErrorCode(uint64_t error);

/* FROM OSDEV WIKI http://wiki.osdev.org/Paging */

/* Bit 0 (P) is the Present flag. */
/* Bit 1 (R/W) is the Read/Write flag. */
/* Bit 2 (U/S) is the User/Supervisor flag. */

/* US RW  P - Description */
/* 0  0  0 - Supervisory process tried to read a non-present page entry */
/* 0  0  1 - Supervisory process tried to read a page and caused a protection fault */
/* 0  1  0 - Supervisory process tried to write to a non-present page entry */
/* 0  1  1 - Supervisory process tried to write a page and caused a protection fault */


/* 1  0  0 - User process tried to read a non-present page entry */
/* 1  0  1 - User process tried to read a page and caused a protection fault */
/* 1  1  0 - User process tried to write to a non-present page entry */
/* 1  1  1 - User process tried to write a page and caused a protection faultUS RW  P - Description */


void handle_pf(uint64_t error){
	printf("page fault handler\n");
	printf("raw error code is ->%p\n",error);
	uint64_t err_code=getErrorCode(error);
	printf("trunc'd error code is ->%p\n",err_code);
	uint64_t pf_va;				/* holds faulting virtual address */
	__asm__  __volatile__(
					   "movq %%cr2,%0"
					   : "=r"(pf_va)
					   :
					   );
	if( (err_code == 7) || (err_code == 5)){
		/* if 111 */
		/* illegal write on existing pages */
		/* which can only be caused by user process writing to kernel pages or writing to his own read only pages  */
		/* segmentation fault */

		/* if 101 */
		/* illegal read on existing pages */
		/* caused by user reading kernel pages */
		/* segmentation fault */
		
		printf("segmentation fault\n");
		/* kill current proc */
		return;
	}
	else if( (err_code == 6) || (err_code == 4) ){
		/* if 110 */
		/* this could be, user writing heap or stack. */
		/* This could be a valid(in heap or near stack vma) or invalid(not in heap and not near stack vma ) */
		/* if valid, then allocate page */
		/* if invalid, segmentation fault */
		
		/* if 100 */
		/* this could be, user reading heap or stack.
		   This could be a valid(in heap or near stack vma) or invalid(not in heap and not near stack vma )  */
		/* if valid, then allocate page frame */
		/* if invalid, segmentation fault */
				
		struct vma *p=proc->vma_head;
		while(p!=NULL){
			/* traverse till you hit stack or heap */


			if(p->flags & (PF_GROWSUP)){
				/* lets consider heap */
				/* if the faulting virt address lies WITHIN heap vma */
				if((p->start <= pf_va ) && ( pf_va < p->end )){
					/* get a new frame and give it to this virt addr */
					/* with perms PT_USER|PT_WRITABLE */
					if(allocuvm(proc->pml4_t,pf_va,1,PT_WRITABLE|PT_USER)==0){
						printf("out of memory...unable to sticth in page for this va..\n");
						/* kill process ?? */
					}
					return;
				}
				/* does not lie in heap vma, continue with stack vma */
			}
			else if(p->flags & (PF_GROWSDOWN)){
				/* lets consider stack */
				if((pf_va < p->end) && (pf_va < p->start)  &&((p->start - pf_va) <= 4)){
					/* if faulting virt address is within 4B of stk_vma.start  */
					/* then allocate frame for stack, increase(lower) stack vma.start  */
					/* give permissions as PT_USER|PT_WRITBALE to page */
					if(allocuvm(proc->pml4_t,pf_va,1,PT_WRITABLE|PT_USER)==0){
						printf("out of memory...unable to esticth in page for this va..\n");
						/* kill process ?? */
					}
					/* extend heap vma by one page downwards*/
					p->start -= FRAME_SIZE;
					return;
				}
				/* sanity check */
				if((pf_va < p->end) && (pf_va >= p->start)){
					printf("page fault within bounds of allocated satck pages....which should not happen\n");
				}
			}
			p=p->next;
		}
		/* if not in any vma's, then invalid */
		/* segmentaion fault */
		printf("segmentation fault\n");		
		/* kill proc  */
		return;
	}	
	printf("unable to match any error in page fault handler\n");

}

inline uint64_t getErrorCode(uint64_t error){
	return (error & 0x7ul);
}
