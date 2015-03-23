#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/utility.h>
#include<sys/sbunix.h>

int vm_init(void* physbase,void* physfree){
	/* alloc 1 frame and clear it for pml4 */
	pml4_base=alloc_frame();
	
	if(!pml4_base){
		printf("unable to allocate frame\n");
		return -1;
	}
	memset1((char *)pml4_base,0,FRAME_SIZE);
	
	/* for page from 0 to 4KB
	   map each page identically*/
	uint64_t i=0;
	for(i=0;i<0x100000ul;i+=0x1000ul){ /* for every page till 1MB */
		if(vm_page_map(i,i,1)==-1){		/* identity map */
			printf("unable to map\n");
			return -1;
		}
	}
	
	/* kernmem = 0xffffffff80000000 + physbase */
	/* for each page from kernmem to kernfree-1 */
	/* map page to range physbase, physfree-1 */
	/* physbase if */
	for(i=(uint64_t)physbase;i<(uint64_t)physfree;i+=0x1000ul){
		if(vm_page_map(i,0xffffffff80000000ul+i,1)==-1){	/* physical addr, virt addr */
			printf("unable to map\n");
			return -1;
		}
	}
	
	/* load pml4_base into crx register. */
	/* 	this will also invalidate tlb */
	load_base((uint64_t)pml4_base);
	return 0;
}


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


int vm_page_map(uint64_t phys_addr,uint64_t virt_addr,int rx_bit){

	/* maps phys_addr to virt_addr */
	/* phys_addr will be base addr of frame */
	/* rx_bit is 1, so that page can be RW  */
	
	//uint64_t pdp_offset=pml4_index(virt_addr);
	



	return -1;
	
}



 void load_base(uint64_t addr){
	 
 }
