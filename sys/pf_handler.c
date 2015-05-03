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
	/* DONT FORGET TO FLUSH TLB */
	uint64_t err_code=getErrorCode(error);
	uint64_t pf_va;				/* holds faulting virtual address */
	__asm__  __volatile__(
					   "movq %%cr2,%0"
					   : "=r"(pf_va)
					   :
					   );
	//printf("proc -> %d -> page fault -> err is %d->pf_va is %p\n",proc->pid,(int)error,pf_va);
	if (err_code==7){
		/* if 111 */
		/* illegal write on existing pages */

		/* ILLEGAL ACCES ON KERNEL VA SPACE*/
		if(pf_va >= KERNBASE){
			printf("segmentation fault\n");
			/* kill proc */
			return ;
		}

		/* flush TLB */
		load_base(get_phys_addr((uint64_t)proc->pml4_t));

		/* COW CODE */
		/* pf_va is virt addr, get pt_entry corresponding to pf_va */
		pt_entry *pt_ent=NULL;
		if((pt_ent=get_pt_entry_for_virt(pf_va))==NULL){
			/* sanity check */
			printf("sanity check failed...unable to get pt_entry for virt addr\n");
		}
		/* pt_ent is the page table entry for this pf_va */
		/* check if cow bit is set */
		if(pt_entry_cow(*pt_ent)){
			/*if, cow is set, COW fork */
			/* check if ref count of the frame is >0 */
			if(get_ref_count(pt_entry_get_frame((*pt_ent))) > 0){
				/* if >0 */
				/* get a new frame  */
				uint64_t *new_frame=(uint64_t *)kmalloc(FRAME_SIZE);
				if(new_frame==NULL){
					printf("unable to allocate memory during COW\n");
					/* kill proc, what should kernel do? */
					return ;
				}
				/* copy the contents of the old frame into new frame */			
				memcpy(new_frame,(void *)get_virt_addr(pt_entry_get_frame((*pt_ent))),FRAME_SIZE);
				/* decr refcount of old frame */
				decr_ref_count(pt_entry_get_frame((*pt_ent)));

				/* clear pt_entry */
				*pt_ent=0x0ul;
				/*  put new frame in pt_ent's frame addr */
				pt_entry_set_frame(pt_ent,get_phys_addr((uint64_t)new_frame));
				/* give WRITE|USER|PRESENT permissions to the pt_entry */
				pt_entry_add_attrib(pt_ent,PT_WRITABLE|PT_USER|PT_PRESENT);
				/* sanity check: ref count of new frame should be 0 */
				if(get_ref_count(pt_entry_get_frame((*pt_ent)))!=0){
					printf("sanity check failed: ref count of new frame should be 0\n");
				}
				/* set ref count of new frame is 0 */				
				set_ref_count(pt_entry_get_frame((*pt_ent)),0);

			}
			else if(get_ref_count(pt_entry_get_frame((*pt_ent))) == 0){
				/* else if ref count is =0 */
				/* this is the onle process referring this frame */
				/* add WRITE bit for this pt_entry, clear COW bit */
				pt_entry_add_attrib(pt_ent,PT_WRITABLE);
				pt_entry_del_attrib(pt_ent,PT_COW);
			}
			else{
				printf("sanity check failed: ref count is < 0\n");
			}
		}
		else{
			/* if, cow is not set,writing to READ ONLY section */
			/* segmentation fault */
			printf("segmentation fault\n");
			/* kill current proc */
		}
		return ;
	}
	else if( (err_code == 5)){
		/* if 101 */
		/* illegal read on existing pages */
		/* caused by user reading kernel pages */
		/* segmentation fault */
		
		printf("segmentation fault\n");
		/* kill current proc */
		return;
	}
	else if( (err_code == 6) || (err_code == 4) || (err_code == 2) ){
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
		/* flush TLB */
		load_base(get_phys_addr((uint64_t)proc->pml4_t));		
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
				if((pf_va < p->end) && (pf_va < p->start)  &&((p->start - pf_va) < STACK_THRESH)){
					/* if faulting virt address is within 64KB of stk_vma.start  */
					/* then allocate frame for stack, increase(lower) stack vma.start  */
					/* give permissions as PT_USER|PT_WRITBALE to page */
					if(allocuvm(proc->pml4_t,pf_va,1,PT_WRITABLE|PT_USER)==0){
						printf("out of memory...unable to esticth in page for this va..\n");
						/* kill process ?? */
					}
					/* extend heap vma by one page downwards*/
					p->start -= STACK_THRESH; /* DO */
					return;
				}
				if((pf_va < p->end) && (pf_va >= p->start)){
					/* USE CASE */
					if(allocuvm(proc->pml4_t,pf_va,1,PT_WRITABLE|PT_USER)==0){
						printf("out of memory...unable to esticth in page for this va..\n");
						/* kill process ?? */
					}
					return ;
				}
			}
			p=p->next;
		}
		/* if not in any vma's, then invalid */
		/* segmentaion fault */
		printf("segmentation fault\n");		
		/* kill proc  */
		for(;;)
			;
		return;
	}	
	printf("unable to match any error in page fault handler\n");
	while(1);
}

inline uint64_t getErrorCode(uint64_t error){
	return (error & 0x7ul);
}
