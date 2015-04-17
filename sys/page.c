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



int check_alloc(pml4 *base,uint64_t offset,uint64_t flags){
	/* returns 0, if failed */
	/* returns 1 if success */
	pd_entry ptr;
	if(!pd_entry_present(base->m_entries[offset])){
		/* pdp entry not present */
		ptr=(pd_entry)alloc_frame(PAGE_SIZE);
		if(!ptr)
			return 0;
		memset1((char *)ptr,0,0x1000);
		pd_entry_set_frame(&base->m_entries[offset],ptr);
		pd_entry_add_attrib(&base->m_entries[offset],PT_PRESENT);
		pd_entry_add_attrib(&base->m_entries[offset],flags);
			
	}
	return 1;
}

int vm_page_map(uint64_t phys_addr,uint64_t virt_addr,uint64_t flags){

	/* maps phys_addr to virt_addr */
	/* phys_addr will be base addr of frame */
	/* rx_bit is 1, so that page can be RW  */

	/* returns 0, if failed */
	/* returns 1 if success */
	
	/* giving write and user permission to pml4, pdp,pd entries of kernel page table */
	/* ASSUME THAT ENTIRE 511 ENTRY OF PML4 IS OFF LIMITS TO USERLAND */
	/* uint64_t temp_flags=PD_WRITABLE|PD_USER; */
	uint64_t temp_flags=PD_WRITABLE;

	/* extract pdp entry from pml4 */
	uint64_t offset=pml4_index(virt_addr); /* offset of pdp in pml4 */
	pml4 *base =(pml4 *)pml4_base;		   /* base of pml4 */
	if(!check_alloc(base,offset,temp_flags))   /* allocs pdp and points pml4+offset to the new pdp frame */
		return 0;
	/* extract pd entry from pdp */
	base=(pml4 *)pd_entry_get_frame(base->m_entries[offset]); /* base of pdp */
	offset=pdp_index(virt_addr);							  /* offset of pd in pdp */
	if(!check_alloc(base,offset,temp_flags))					  /* allocs pd and points pdp+offset to new pd */
		return 0;
	/* extract pt entry from pd */
	base=(pml4 *)pd_entry_get_frame(base->m_entries[offset]); /* base of pd */
	offset=pd_index(virt_addr);								  /* offset of pt in pd */
	if(!check_alloc(base,offset,temp_flags))					  /* allocs pt and points pd+offset to new pt */
		return 0;
	/* extract page entry from pt */
	base=(pml4 *)pd_entry_get_frame(base->m_entries[offset]); /* base of pt */
	offset=pt_index(virt_addr);								  /* offset of page in pt */
	/* put frame address of phys_addr into the frame of page table entry */
	pd_entry ptr=phys_addr;
	pd_entry_set_frame(&base->m_entries[offset],ptr);
	pd_entry_add_attrib(&base->m_entries[offset],PT_PRESENT);
	pd_entry_add_attrib(&base->m_entries[offset],flags);

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
	pml4_base=alloc_frame(PAGE_SIZE);
	
	if(!pml4_base){
		printf("unable to allocate frame\n");
		return 0;
	}
	memset1((char *)pml4_base,0,FRAME_SIZE);
	
	/* 511 entry of pml4 of any user table is of limits to user */
	/* pml4 pdp, pd and pt of kernel space all have write and supervisor perms */
	uint64_t flags=PT_WRITABLE;
	/* for page from 0 to 4KB
	   map each page identically*/
	uint64_t i=0;
	for(i=0;i<0x100000ul;i+=0x1000ul){ /* for every page till 1MB */
		//printf("%p maps to ",i);
		if(!vm_page_map(i,i,flags)){		/* identity map */
			printf("unable to map\n");
			return 0;
		}
	}
	
	/* kernmem = 0xffffffff80000000 + physbase */
	/* for each page from kernmem to kernfree-1 */
	/* map page to range physbase, physfree-1 */
	/* physbase if */
	for(i=(uint64_t)0x000000ul;i<(uint64_t)0x7ffe000ul;i+=0x1000ul){
		if(!vm_page_map(i,0xffffffff80000000ul+i,flags)){	/* physical addr, virt addr */
			printf("unable to map\n");
			return 0;
		}
	}
	
	/* load pml4_base into crx register. */
	/* 	this will also invalidate tlb */
	load_base((uint64_t)pml4_base);
	return 1;
}

int u_check_alloc(pml4 *base,uint64_t offset,uint64_t flags){
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
		pd_entry_add_attrib(&base->m_entries[offset],flags);
		
			
	}
	return 1;
}



int u_alloc_frame_for_va(pml4 *pml4_t,uint64_t virt_addr,uint64_t flags){

	/* return 0 on failure */
	/* return 1 on success */
	/* returns 1 if, a physical frame was already alloc'd for this VA */
	/* if not alloc'd, creates a frame for this VA and then returns 1  */
	uint64_t offset=pml4_index(virt_addr); /* offset for pml4 entry */
	pml4 *base=pml4_t;
	/* pml4, pdp, pd entries should have writable, user bits set */
	/* pml4's 511 entry is already set as supervisor and writable */
	uint64_t temp_flags=PD_WRITABLE|PD_USER;
	/* PML4 falgs set as writable, user */
	if(!u_check_alloc(base,offset,temp_flags)){
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
	if(!u_check_alloc(base,offset,temp_flags)){
		/* u_check failed */
		return 0;
	}
	/* now pd frame was allocated and stitched into pdp */
	/* get virt addr of pd */
	base=(pml4 *)get_virt_addr(pd_entry_get_frame(base->m_entries[offset]));
	/* get pd offset */
	offset=pd_index(virt_addr);
	if(!u_check_alloc(base,offset,temp_flags)){
		/* u_check failed */
		return 0;
	}
	/* now pt frame was alloc'd and stitched into pd */
	/* get virt adr of pt */
	base=(pml4 *)get_virt_addr(pd_entry_get_frame(base->m_entries[offset]));
	/* get pt offset */
	offset=pt_index(virt_addr);
	if(!u_check_alloc(base,offset,flags)){
		/* u_check failed */
		return 0;
	}
	/* now a frame was allocated and stitched into pt as en entry */
	/* this was the last level */
	/* we already cleared the physical frame  */
	return 1;
	
}



int allocuvm(pml4 *pml4_t,uint64_t virt_addr,uint64_t sz,uint64_t flags){
	/* return 0 on failure */
	/* return sz on success */
	/* allocates physical frames for the VA range virt_addr to (virt_addr+sz) */
	uint64_t i=0;
	for(i=virt_addr;i<virt_addr+sz;i++){
		/* alloc a frame for this  addr i*/
		if(!u_alloc_frame_for_va(pml4_t,i,flags)){
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
	/* all pml4/pd pointers passed to this function are virt addrs */
	int i=0;
	for(;i<511;i++){
		/* if an entry of pml4 is present */
		if(pd_entry_present(pml4_t->m_entries[i])){
			/* then free it's pdp table */
			free_pdp((pdp *)(get_virt_addr(pd_entry_get_frame(pml4_t->m_entries[i]))));
			/* pdp table was freed withing free_pdp */
		}
		/* else continue */
	}
	/* kfree pml4 table */
	kfree(pml4_t);
}

void free_pdp(pdp *pdp_t){
	/* call free_pd on each entry of pdp */
	int i=0;
	for(;i<512;i++){
		/* if an entry of pml4 is present */
		if(pd_entry_present(pdp_t->m_entries[i])){
			/* then free it's pdp table */
			free_pd((pd *)(get_virt_addr(pd_entry_get_frame(pdp_t->m_entries[i]))));
			/* pdp table was freed */
		}
		/* else continue */
	}
	
	/* finally free the pdp table */
	kfree(pdp_t);
}

void free_pd(pd *pd_t){
	/* call free_pd on each entry of pd */
	int i=0;
	for(;i<512;i++){
		/* if an entry of pd is present */
		if(pd_entry_present(pd_t->m_entries[i])){
			/* then free it's pt table */
			free_pt((pt *)(get_virt_addr(pd_entry_get_frame(pd_t->m_entries[i]))));
			/* pd table was freed */
		}
		/* else continue */
	}
	
	/* finally free the pd table */
	kfree(pd_t);
}

void free_pt(pt *pt_t){
	/* call free_pd on each entry of pd */
	int i=0;
	for(;i<512;i++){
		/* if an entry of pd is present */
		if(pt_entry_present(pt_t->m_entries[i])){
			/* then kfree it's physical frame */
			kfree((void *)get_virt_addr(pt_entry_get_frame(pt_t->m_entries[i])));
		}
		/* else continue */
	}
	
	/* finally free the pd table */
	kfree(pt_t);
}


pml4 * copyuvm(pml4 *parent_ml4_t){
	/* return NULL if fail */
	/* return address of pml4 if success */
	return NULL;
}
