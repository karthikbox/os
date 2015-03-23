#ifndef __PAGE_H
#define __PAGE_H

#include <sys/defs.h>

/* level 1 */
enum PAGE_PTE_FLAGS{

	PTE_PRESENT = 1,//1
	PTE_WRITABLE = 2,
	PTE_USER = 4,//3
	PTE_WRITETHROUGH =8,
	PTE_NOT_CACHEABLE=0x10,
	PTE_ACCESSED=0x20,//6
	PTE_DIRTY=0x40,
	PTE_NULL=0x80,//8
	PTE_CPU_GLOBAL=0x100,
	PTE_LV4_GLOBAL=0x200,//10
	PTE_FRAME=0x0000FFFFFFFFF000//36 bit frame number [12-47]
};

/*page table entry */
typedef uint64_t pt_entry;

void pt_entry_add_attrib(pt_entry *e, uint64_t attrib);
void pt_entry_del_attrib(pt_entry *e, uint64_t attrib);
void pt_entry_set_frame(pt_entry *e, uint64_t addr);
int pt_entry_is_present(pt_entry e);
int pt_entry_is_writable(pt_entry e);
int pt_entry_is_user(pt_entry e);
uint64_t pt_entry_pfn(pt_entry e);

enum PAGE_PD_FLAGS{

	PD_PRESENT = 1,//1
	PD_WRITABLE = 2,
	PD_USER = 4,//3
	PD_WRITETHROUGH =8,
	PD_NOT_CACHEABLE=0x10,
	PD_ACCESSED=0x20,//6
	PD_DIRTY=0x40,
	PD_NULL=0x80,//8
	PD_CPU_GLOBAL=0x100,
	PD_LV4_GLOBAL=0x200,//10
	PD_FRAME=0x0000FFFFFFFFF000//36 bit frame number [12-47]
};

typedef uint64_t pd_entry;

void pd_entry_add_attrib(pd_entry *e, uint64_t attrib);
void pd_entry_del_attrib(pd_entry *e, uint64_t attrib);
void pd_entry_set_frame(pd_entry *e, uint64_t addr);
int pd_entry_is_present(pd_entry e);
int pd_entry_is_writable(pd_entry e);
int pd_entry_is_user(pd_entry e);
int pd_entry_is_4mb(pd_entry e);
uint64_t pd_entry_pfn(pd_entry e);
int pd_entry_is_enable_global(pd_entry e);



#define ENTRIES_PER_PT 512
#define ENTRIES_PER_PD 512
#define ENTRIES_PER_PDP 512
#define ENTRIES_PER_PML4 512

#define PML4_INDEX(x) ((x>>39) & 0x1ff) /* 9 bits */
#define PDP_INDEX(x) ((x>>30) & 0x1ff)
#define PD_INDEX(x) ((x>>21) & 0x1ff)
#define PT_INDEX(x) ((x>>12) & 0x1ff)
#define LOWER_OFFSET(x) (x & 0xfff) /* 12 bits */


#define PAGE_SIZE 4096
/* pd, pdp, pml4 are ientical. so pd_entry holds for the three of these */
/* pt_entry holds pt entries, since it is slightly different from the above */
struct pt{
	pt_entry m_entries[ENTRIES_PER_PT];
};

struct pd{
	pd_entry m_entries[ENTRIES_PER_PD];
};

struct pdp{
	pd_entry m_entries[ENTRIES_PER_PDP];
};

struct pml4{
	pd_entry m_entries[ENTRIES_PER_PML4];
};

int vm_alloc_page(pt_entry* e);
void vm_free_page(pt_entry* e);
void mv_map_page(void* phys,coid *virt);
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
