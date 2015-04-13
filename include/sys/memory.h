#include <sys/defs.h>

struct frame_manager
{
	void* frame_start_addr;
	struct frame_manager *next;
	size_t offset;//8bytes
	size_t size;//8 bytes
	size_t free;//8 bytes
};


typedef struct frame_manager *p_fmgr;

p_fmgr head;//head points to the head of the memory linked list
p_fmgr tail;//tail points to the last node of the linked list, brk is data chunk + meta
void* page_phys_addr;
p_fmgr frame_manager_last;
p_fmgr frame_manager_start;

#define FRAME_MGR_SIZE 40
#define ALIGNMENT 8
#define ALIGN(x) (((x)+(ALIGNMENT-1)) & ~ (ALIGNMENT-1))
#define ENTRIES_PER_FRAME_MGR 102
//#define NUMBER_OF_FRAMES(size) (size%4096==0?size/4096:size/4096+1) 
void* kmalloc(size_t size);
void kfree(void* ptr);
void* alloc_addr(size_t size);
void* new_page_mgr_alloc(size_t size);
void* init_page(p_fmgr node,size_t size);
void* add_mgr_node(p_fmgr node, size_t size);
