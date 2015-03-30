#include <sys/pmmgr.h>
#include <sys/sbunix.h>
#include <sys/process.h>

void* kmalloc(size_t size){

	if(size > FRAME_SIZE){
		printf("kmalloc: Requested size is larger than the FRAME_SIZE\n");
		return NULL;
	}

	//get the physical address of the first available free frame
	void* free_frame_phys_addr = alloc_frame();

	//map the physical address to the corresponding virtual address and return
	uint64_t free_frame_virt_addr = get_virt_addr((uint64_t)free_frame_phys_addr);
	return (void*) free_frame_virt_addr;
}

void kfree(void* addr){

	//get the physical address of the corresponding virtual address and free the frame
	uint64_t frame_phys_addr = get_phys_addr((uint64_t)addr);
	free_frame((void*) frame_phys_addr);

}