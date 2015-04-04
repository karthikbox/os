#include <sys/pmmgr.h>
#include <sys/sbunix.h>
#include <sys/process.h>
#include <sys/kmalloc.h>

void* kmalloc(size_t size)
{
	/* returns NULL on failure */
	/* todo: mark kmalloc data structure memory as used in pmmgr and never free it(?) */
	//printf("In kmalloc()\n");
	p_blk temp;
	size=ALIGN(size);
	printf("Size requested is %d\n", size);
	if(size==0)
	{
		//refer man page of malloc
		return NULL;
    }

    if(!head)
    {
		//if head is null, malloc has no memory, expand brk
		//set head to meta ptr returned by expand_brk
		//since head is the tail also, tail=head
		temp=expand_brk(size);
		if(temp==NULL)
		{
	    	//expand_brk failed, return null
	    	return NULL;
		}
		head=temp;
		return (void*) head->start;	
    }
	else
	{
		temp=get_mem_node(size);
		if(temp)
		{
		    //temp points to some memory meta node
		    //this chunk has enough space for size
		    if(can_split(temp,size))
		    {
				//if this chunk is too big, split it into two chunks
				//split(temp) splits temp into 2 chunks and uodates the linked list
				split(temp,size);
	    	}
	    temp->free=0;
	    return temp->start;
		}
		else
		{
		    //temp does not point to any node
		    //no memory node is free or large enough to fit size
		    //expand brk to create this new chunk
		    //return the new chunk
		    //expand_brk also takes care of the lnked list, i.e updates tail
		    temp=expand_brk(size);
	    	if(temp==NULL)
				return NULL;//expand brk failed, return NULL
	    	return temp->start;
		}
    }




	/*if(size > FRAME_SIZE){
		printf("kmalloc: Requested size is larger than the FRAME_SIZE\n");
		return NULL;
	}

	//get the physical address of the first available free frame
	void* free_frame_phys_addr = alloc_frame();

	//map the physical address to the corresponding virtual address and return
	uint64_t free_frame_virt_addr = get_virt_addr((uint64_t)free_frame_phys_addr);
	return (void*) free_frame_virt_addr;*/
}



void kfree(void* ptr){

	//get the physical address of the corresponding virtual address and free the frame
	//uint64_t frame_phys_addr = get_phys_addr((uint64_t)addr);
	//free_frame((void*) frame_phys_addr);
	printf("Address to be freed is %p\n", ptr);
	if(valid_ptr(ptr))
	{
		//printf("In kfree\n");
		
		p_blk temp=get_meta_ptr(ptr);
		temp->free=1;
		if(temp->next==NULL)
		{
		    //last block is being free'd
		    //decrease the brk pointer to temp
		    //sanity check
		    if(temp!=tail)
				printf("last node to remove is not same as tail\n");

	    	tail=temp->prev;
	    	if(tail!=NULL)
	    	{
				tail->next=NULL;
	    	}
	    	else
	    	{
				head=tail;
	    	}
	    	if(!remove_last_blk(temp))
				printf("unable to remove last blk, just marking it as free\n");;
		}
	/*
	  TODO
	  1.implement coalascing
	  2.check prev and next block if they are free then coalasce
	  into 1 big block
	 */
    }

}

p_blk get_meta_ptr(void *ptr)
{
    char *temp=ptr;
    temp-=M_BLK_SIZE;
    return (p_blk)temp;
}

int remove_last_blk(p_blk t)
{
    /*if(brk((void *)t)==-1)
    {
		printf("unable to decrease brk to remove last blk\n");
		return 0;
    }*/
	//printf("brk_ptr before in remove_last_blk() is %p\n", brk_ptr);
	brk_ptr = (void*)((uint64_t)brk_ptr - t->size - M_BLK_SIZE);
	//printf("brk_ptr after in remove_last_blk() is %p\n", brk_ptr);
    return 1;
}

int valid_ptr(void *ptr)
{
	//printf("In valid_ptr()\n");
    p_blk p=ptr;
    //printf("Condition : %d\n", p>=head);
    //printf("Condition : %d\n", (p < (p_blk)get_virt_addr((uint64_t)sbrk(0))));
    //printf("sbrk(0) is %p\n", sbrk(0));
    if(head && ptr &&  (p >=head) && (p < (p_blk) get_virt_addr((uint64_t)sbrk(0))) ) 
    {
    	
		if( p == (get_meta_ptr(ptr))->ptr )
		{
	    	return 1;
		}
    }
    return 0;
}

p_blk expand_brk(size_t size){
    //expand_brk raises brk by size+metablock
    //updates tail to new metanode, if head is not null
    //returns tail
    //returns NULL on error
    //printf("In expand_brk()\n");
    p_blk temp;
    temp=sbrk(0);
    temp=(p_blk)get_virt_addr((uint64_t) temp);
    //printf("Temp is %p\n", temp);
    if(sbrk(size+M_BLK_SIZE)==(void *)-1)
    {
		//sbrk failed
		return NULL;
    }
    if(head)
    {
		//head is not NULL, list already initialized	
		tail->next=temp;
		temp->prev=tail;
		tail=temp;
		tail->size=size;
		tail->free=0;
		tail->next=NULL;
		tail->ptr=tail->start;
    }
    else
    {
		//head is null
		//no memory on heap
		tail=temp;
		tail->size=size;
		tail->free=0;
		tail->next=NULL;
		tail->prev=NULL;
		tail->ptr=tail->start;
    }
    //printf("Before return\n");
    return temp;
}


void* sbrk(uint64_t offset){
    //assuming NULL is invalid address, sys_brk return the current brk pointer
    //sbrk will return cur_brk, whatever the offset
    // heap brk decreases if offset is negative
    // sbrk or brk will not decrease brk below the default brk set by kernel
    //when program is loaded
	//printf("In sbrk()\n");
	//printf("Offset in sbrk is %d\n", offset);
    if(first_alloc)
    {
    	first_alloc = 0;
    	void* free_frame_phys_addr = alloc_frame();
    	//printf("physical address of first frame is %p\n", (uint64_t)free_frame_phys_addr);
    	brk_ptr = (void*)(uint64_t)free_frame_phys_addr;
    }

    //printf("brk_ptr before is %p\n", brk_ptr);
    void* cur_brk = brk_ptr;
    brk_ptr = (void*)(uint64_t)brk_ptr + (uint64_t)offset;

    //printf("brk_ptr after is %p\n", brk_ptr);
    /*void * cur_brk = (void *) syscall_1(SYS_brk,(uint64_t)NULL);
    void * new_brk=(void *)((uint64_t)cur_brk+(uint64_t)offset);
    if(brk(new_brk)==-1){
	//brk() did not set brk pointer to new_brk
	return (void *)-1;
    }
    else{
	//brk() set the brk pointer to new_brk
	return cur_brk;
    }*/
    return cur_brk;
    
}


p_blk get_mem_node(size_t size)
{
    p_blk t=head;
    while(t!=NULL)
    {
		if(t->free==1)
		{
	    	if( (t->size) >= size )
	    	{
				//t size is large enough to fit size
				return t;
	    	}
		}
		t=t->next;
    }
    //no free block or no free block big enough
    return NULL;
}

int can_split(p_blk t,size_t size)
{
    if( (t->size - size) >= (M_BLK_SIZE+MIN_SPLIT_SIZE))
		return 1;
    else
		return 0;
}

void split(p_blk t,size_t size)
{
    p_blk temp;
    temp=(p_blk)(t->start+size);
    temp->size=t->size-M_BLK_SIZE-size;
    temp->free=1;
    temp->ptr=temp->start;
    temp->next=t->next;
    temp->prev=t;

    t->next->prev=temp;

    t->next=temp;
    t->size=size;
    t->free=0;

    
}
