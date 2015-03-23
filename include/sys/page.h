#ifndef __PAGE_H
#define __PAGE_H

#include <sys/defs.h>




/* level 1 */
enum PAGE_PTE_FLAGS{

	PTE_PRESENT = 1ul,//1
	PTE_WRITABLE = 2ul,
	PTE_USER = 4ul,//3
	PTE_WRITETHROUGH =8ul,
	PTE_NOT_CACHEABLE=0x10ul,
	PTE_ACCESSED=0x20ul,//6
	PTE_DIRTY=0x40ul,
	PTE_NULL=0x80ul,//8
	PTE_CPU_GLOBAL=0x100ul,
	PTE_LV4_GLOBAL=0x200ul,//10
	PTE_FRAME=0x0000FFFFFFFFF000ul//36 bit frame number [12-47]
};

/*page table entry */
typedef uint64_t pt_entry;

void pt_entry_add_attrib(pt_entry *e, uint64_t attrib);
void pt_entry_del_attrib(pt_entry *e, uint64_t attrib);
void pt_entry_set_frame(pt_entry *e, uint64_t addr);
int pt_entry_is_present(pt_entry e);
int pt_entry_is_writable(pt_entry e);
int pt_entry_is_user(pt_entry e);
uint64_t pt_entry_get_frame(pt_entry e);

enum PAGE_PD_FLAGS{

	PD_PRESENT = 1ul,//1
	PD_WRITABLE = 2ul,
	PD_USER = 4ul,//3
	PD_WRITETHROUGH =8ul,
	PD_NOT_CACHEABLE=0x10ul,
	PD_ACCESSED=0x20ul,//6
	PD_DIRTY=0x40ul,
	PD_NULL=0x80ul,//8
	PD_CPU_GLOBAL=0x100ul,
	PD_LV4_GLOBAL=0x200ul,//10
	PD_FRAME=0x0000FFFFFFFFF000ul//36 bit frame number [12-47]
};

typedef uint64_t pd_entry;

void pd_entry_add_attrib(pd_entry *e, uint64_t attrib);
void pd_entry_del_attrib(pd_entry *e, uint64_t attrib);
void pd_entry_set_frame(pd_entry *e, uint64_t addr);
int pd_entry_is_present(pd_entry e);
int pd_entry_is_writable(pd_entry e);
int pd_entry_is_user(pd_entry e);
int pd_entry_is_4mb(pd_entry e);
uint64_t pd_entry_get_frame(pd_entry e);
int pd_entry_is_enable_global(pd_entry e);



#define ENTRIES_PER_PT 512
#define ENTRIES_PER_PD 512
#define ENTRIES_PER_PDP 512
#define ENTRIES_PER_PML4 512

inline uint64_t pml4_index(uint64_t x);
inline uint64_t pdp_index(uint64_t x);
inline uint64_t pd_index(uint64_t x);
inline uint64_t pt_index(uint64_t x);
inline uint64_t lower_offset(uint64_t x);

#define PAGE_SIZE 4096
/* pd, pdp, pml4 are ientical. so pd_entry holds for the three of these */
/* pt_entry holds pt entries, since it is slightly different from the above */
typedef struct{
	pt_entry m_entries[ENTRIES_PER_PT];
}pt;

typedef struct{
	pd_entry m_entries[ENTRIES_PER_PD];
}pd;

typedef struct{
	pd_entry m_entries[ENTRIES_PER_PDP];
}pdp;

typedef struct{
	pd_entry m_entries[ENTRIES_PER_PML4];
}pml4;


/* holds the base address of pml4 frame */
pml4 *pml4_base;

int vm_alloc_page(pt_entry* e);
void vm_free_page(pt_entry* e);
int vm_page_map(uint64_t phys,uint64_t virt,int rx_bit);
void load_base(uint64_t addr);
/*
1234 5678 
0000 0000 1 00
0000 1111 2 0F
1111 1111 3 FF
1111 1111 4 FF
1111 1111 5 FF
1111 1111 6 FF
XXXX XXXX 7 00
XXXX XXXX 8 00
*/

#endif
