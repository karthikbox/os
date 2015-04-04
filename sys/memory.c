#include <sys/kmalloc.h>
#include <sys/pmmgr.h>
#include <sys/sbunix.h>
#include <sys/process.h>
#include <sys/page.h>

p_fmgr head=NULL;//head points to the head of the memory linked list
p_fmgr tail=NULL;//tail points to the last node of the linked list, brk is data chunk + meta
void* page_phys_addr = NULL;
p_fmgr frame_manager_last = NULL;
p_fmgr frame_manager_start = NULL;


void* kmalloc(size_t size)
{
	//allign the size in multiples of 8
	size = ALIGN(size);
	void* ret_addr = alloc_addr(size);

	return ret_addr;
	
}

void* alloc_addr(size_t size)
{
	p_fmgr temp = NULL, prev = NULL;
	void* ret_addr = NULL;

	if(!head)
	{
		//allocate a page for the frame_manager	to store the metadata of the malloc'ed pages
		head = new_page_mgr_alloc();
		frame_manager_start = head;
		printf("Head is %p\n", head);

		//frame_manager_last tracks the last metadata
		frame_manager_last = head;

		//creates the actual page and adds the metadata to the head
		init_page(head);

		//allocate size to the page and update the metadata in the frame_manager
		add_mgr_node(head, size);

		ret_addr = (void*)((uint64_t)head->frame_start_addr + (uint64_t)head->offset);
		return ret_addr;
	}

	//traverse the frame_manager list and check if we can find a chunk with 'size' size.
	temp = head;
	while(temp != NULL)
	{
		if((temp->free == 1) && temp->size >= size)
		{
			add_mgr_node(temp, size);
			ret_addr = (void*)((uint64_t)temp->frame_start_addr + (uint64_t)temp->offset);

			return ret_addr;
		}
		prev = temp;
		temp = temp->next;
	}

	//we couldn't allocate a chunk in the existing frame_manager list
	//so create a new page and add to the existing frame_manager lise
	p_fmgr node = frame_manager_last + 1;
	prev->next = node;
	frame_manager_last = node;

	init_page(node);
	add_mgr_node(node, size);
	ret_addr = (void*)((uint64_t)node->frame_start_addr + (uint64_t)node->offset);

	return ret_addr;
}

void init_page(p_fmgr node)
{
	page_phys_addr = (void*) get_virt_addr((uint64_t)alloc_frame());

	node->frame_start_addr = page_phys_addr;
	node->offset = 0;
	node->next = NULL;
	node->free = 1;
	node->size = PAGE_SIZE;
}

void add_mgr_node(p_fmgr node, size_t size)
{
	p_fmgr new_node;
	
	//check if frame_manager is full or not
	if((frame_manager_start + ENTRIES_PER_FRAME_MGR) == frame_manager_last)
	{
		frame_manager_start = new_page_mgr_alloc();
		frame_manager_last->next = frame_manager_start;
		new_node = frame_manager_start;
	}
	else
	{
		//create a new node at the end of the frame_manager list and update the frame_manager_last
		new_node = frame_manager_last + 1;	
	}
	
	frame_manager_last = new_node;

	new_node->frame_start_addr = node->frame_start_addr;
	new_node->offset = node->offset + size;
	new_node->next = node->next;
	new_node->size = node->size - size;
	new_node->free = 1;

	node->free = 0;
	node->size = size;
	node->next = new_node;
}

void* new_page_mgr_alloc(p_fmgr node)
{
	void* frame_manager_phys_addr = alloc_frame();
	node = (p_fmgr)get_virt_addr((uint64_t) frame_manager_phys_addr);
	printf("new_page_mgr address is %p\n", node);

	return (void*)node;
}


void kfree(void* addr)
{
	p_fmgr temp;
	temp = head;

	while(temp != NULL)
	{
		if((void*)((uint64_t)temp->frame_start_addr + (uint64_t)temp->offset) == addr)
		{
			temp->free = 1;
			break;
		}
		temp = temp->next;
	}
}
