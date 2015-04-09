#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/utility.h>
#include<sys/sbunix.h>
#include<sys/process.h>
inline uint64_t pml4_index(uint64_t x){
	return ((x>>39) & 0x1fful);
}

inline uint64_t pdp_index(uint64_t x){
	return ((x>>30) & 0x1fful);
}

inline uint64_t pd_index(uint64_t x){
	return ((x>>21) & 0x1fful);
}

inline uint64_t pt_index(uint64_t x){
	return ((x>>12) & 0x1fful);
}

inline uint64_t lower_offset(uint64_t x){
	return ( x & 0x0ffful);
}

inline int pd_entry_present(pd_entry e){
	return (e & PD_PRESENT);			/* returns 1 if P is set, otherwise 0 */
}

inline uint64_t pd_entry_get_frame(pd_entry e){
	return (e & PD_FRAME);			
}

inline void pd_entry_set_frame(pd_entry *e,uint64_t addr){
	
	*e = addr | (*e & 0xffful); /* the first 12 bits of addr will be 0 since addr is 4KB aligned */
	
}


inline void pd_entry_add_attrib(pd_entry *e,uint64_t attrib){
	*e=*e|attrib;
}

inline void pd_entry_del_attrib(pd_entry *e,uint64_t attrib){
	*e=*e&(~(attrib));
}

inline void pt_entry_add_attrib(pt_entry *e,uint64_t attrib){
	*e=*e|attrib;
}

inline void pt_entry_del_attrib(pt_entry *e,uint64_t attrib){
	*e=*e&(~(attrib));
}

inline int pt_entry_present(pt_entry e){
	return (e & PT_PRESENT);			/* returns 1 if P is set, otherwise 0 */
}

inline uint64_t pt_entry_get_frame(pt_entry e){
	return (e & PT_FRAME);			
}

inline void pt_entry_set_frame(pt_entry *e,uint64_t addr){
	
	*e = addr | (*e & 0xffful); /* the first 12 bits of addr will be 0 since addr is 4KB aligned */
	
}



int check_alloc(pml4 *base,uint64_t offset,int rx_bit){
	/* returns 0, if failed */
	/* returns 1 if success */
	pd_entry ptr;
	if(!pd_entry_present(base->m_entries[offset])){
		/* pdp entry not present */
		ptr=(pd_entry)alloc_frame();
		if(!ptr)
			return 0;
		memset1((char *)ptr,0,0x1000);
		pd_entry_set_frame(&base->m_entries[offset],ptr);
		pd_entry_add_attrib(&base->m_entries[offset],PT_PRESENT);
		if(rx_bit){
			/* rx_bit is 1, set RW bit */
			pd_entry_add_attrib(&base->m_entries[offset],PT_WRITABLE);
		}
			
	}
	return 1;
}

int vm_page_map(uint64_t phys_addr,uint64_t virt_addr,int rx_bit){

	/* maps phys_addr to virt_addr */
	/* phys_addr will be base addr of frame */
	/* rx_bit is 1, so that page can be RW  */

	/* returns 0, if failed */
	/* returns 1 if success */
	

	/* extract pdp entry from pml4 */
	uint64_t offset=pml4_index(virt_addr); /* offset of pdp in pml4 */
	pml4 *base =(pml4 *)pml4_base;		   /* base of pml4 */
	if(!check_alloc(base,offset,1))   /* allocs pdp and points pml4+offset to the new pdp frame */
		return 0;
	/* extract pd entry from pdp */
	base=(pml4 *)pd_entry_get_frame(base->m_entries[offset]); /* base of pdp */
	offset=pdp_index(virt_addr);							  /* offset of pd in pdp */
	if(!check_alloc(base,offset,1))					  /* allocs pd and points pdp+offset to new pd */
		return 0;
	/* extract pt entry from pd */
	base=(pml4 *)pd_entry_get_frame(base->m_entries[offset]); /* base of pd */
	offset=pd_index(virt_addr);								  /* offset of pt in pd */
	if(!check_alloc(base,offset,1))					  /* allocs pt and points pd+offset to new pt */
		return 0;
	/* extract page entry from pt */
	base=(pml4 *)pd_entry_get_frame(base->m_entries[offset]); /* base of pt */
	offset=pt_index(virt_addr);								  /* offset of page in pt */
	/* put frame address of phys_addr into the frame of page table entry */
	pd_entry ptr=phys_addr;
	pd_entry_set_frame(&base->m_entries[offset],ptr);
	pd_entry_add_attrib(&base->m_entries[offset],PT_PRESENT);
	if(rx_bit){
		/* rx_bit is 1, set RW bit */
		/* basically rx_bit is used to set the RW flag of only the page */
		/* all the higher structeres are given RW as 1 by default */
		pd_entry_add_attrib(&base->m_entries[offset],PT_WRITABLE);
	}
	//printf("%p\n",base->m_entries[offset]);
	/* success */
	return 1;
	
}



 void load_base(uint64_t addr){
	 __asm volatile(
					"movq %0,%%cr3"
					: 
					: "r"(addr)
					);
 }


int vm_init(void* physbase,void* physfree){
	/* alloc 1 frame and clear it for pml4 */
	pml4_base=alloc_frame();
	
	if(!pml4_base){
		printf("unable to allocate frame\n");
		return 0;
	}
	memset1((char *)pml4_base,0,FRAME_SIZE);
	
	/* for page from 0 to 4KB
	   map each page identically*/
	uint64_t i=0;
	for(i=0;i<0x100000ul;i+=0x1000ul){ /* for every page till 1MB */
		//printf("%p maps to ",i);
		if(!vm_page_map(i,i,1)){		/* identity map */
			printf("unable to map\n");
			return 0;
		}
	}
	
	/* kernmem = 0xffffffff80000000 + physbase */
	/* for each page from kernmem to kernfree-1 */
	/* map page to range physbase, physfree-1 */
	/* physbase if */
	for(i=(uint64_t)0x100000ul;i<(uint64_t)0x7ffe000ul;i+=0x1000ul){
		if(!vm_page_map(i,0xffffffff80000000ul+i,1)){	/* physical addr, virt addr */
			printf("unable to map\n");
			return 0;
		}
	}
	
	/* load pml4_base into crx register. */
	/* 	this will also invalidate tlb */
	load_base((uint64_t)pml4_base);
	return 1;
}

int u_check_alloc(pml4 *base,uint64_t offset,int rx_bit){
	/* returns 0, if failed */
	/* returns 1 if success */
	pd_entry ptr;
	if(!pd_entry_present(base->m_entries[offset])){
		/* pdp entry not present */
		ptr=(pd_entry)kmalloc(FRAME_SIZE); /* kernel VA */
		if(!ptr)
			return 0;
		memset1((char *)ptr,0,0x1000); /* clear contents */
		/* convert KVA to physical addr */
		/*  */
		pd_entry_set_frame(&base->m_entries[offset],get_phys_addr((uint64_t)ptr));
		pd_entry_add_attrib(&base->m_entries[offset],PT_PRESENT);
		if(rx_bit){
			/* rx_bit is 1, set RW bit */
			pd_entry_add_attrib(&base->m_entries[offset],PT_WRITABLE);
		}
			
	}
	return 1;
}



int u_alloc_frame_for_va(pml4 *pml4_t,uint64_t virt_addr){

	/* return 0 on failure */
	/* return 1 on success */
	/* returns 1 if, a physical frame was already alloc'd for this VA */
	/* if not alloc'd, creates a frame for this VA and then returns 1  */
	uint64_t offset=pml4_index(virt_addr); /* offset for pml4 entry */
	pml4 *base=pml4_t;
	if(!u_check_alloc(base,offset,1)){
		/* u_check_failed */
		return 0;
	}
	/* now a pdp frame was allocated and stitched into pml4 */
	/* get addr of pdp */
	/* offset is still offset into pml4 */
	/* pd_entry_get_frame returns phys addr. convert it into virt addr */
	base=(pml4 *)get_virt_addr(pd_entry_get_frame(base->m_entries[offset]));
	/* get new offset in pdp table */
	offset=pdp_index(virt_addr);
	if(!u_check_alloc(base,offset,1)){
		/* u_check failed */
		return 0;
	}
	/* now pd frame was allocated and stitched into pdp */
	/* get virt addr of pd */
	base=(pml4 *)get_virt_addr(pd_entry_get_frame(base->m_entries[offset]));
	/* get pd offset */
	offset=pd_index(virt_addr);
	if(!u_check_alloc(base,offset,1)){
		/* u_check failed */
		return 0;
	}
	/* now pt frame was alloc'd and stitched into pd */
	/* get virt adr of pt */
	base=(pml4 *)get_virt_addr(pd_entry_get_frame(base->m_entries[offset]));
	/* get pt offset */
	offset=pt_index(virt_addr);
	if(!u_check_alloc(base,offset,1)){
		/* u_check failed */
		return 0;
	}
	/* now a frame was allocated and stitched into pt as en entry */
	/* this was the last level */
	/* we already cleared the physical frame  */
	return 1;
	
}



int allocuvm(pml4 *pml4_t,uint64_t virt_addr,uint64_t sz){
	/* return 0 on failure */
	/* return sz on success */
	/* allocates physical frames for the VA range virt_addr to (virt_addr+sz) */
	uint64_t i=0;
	for(i=virt_addr;i<virt_addr+sz;i++){
		/* alloc a frame for this  addr i*/
		if(!u_alloc_frame_for_va(pml4_t,i)){
			/* alloc_frame_for_va failed */
			printf("alloc_frame_for_va failed\n");
			return 0;
		}
	}
	return sz;					/* success. all of range got alloc'd phys frames */
}

void free_uvm(pml4 *pml4_t){
	/* deallocates the pml4 entries except fpr 511 entry recursively */
	/* finally frees up pml4 table memory */
	;							/* TODO */
}


