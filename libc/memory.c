#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/syscall.h>


int brk(void *end_data_segment){
    void * ret = (void *) syscall_1(SYS_brk,(uint64_t)end_data_segment);
    if((uint64_t)ret==(uint64_t)end_data_segment){
	printf("success1\n");
	return 0;
    }
    else{
	return -1;
    }    
}

void * sbrk(uint64_t offset){
    //assuming NULL is invalid address, sys_brk return the current brk pointer
    //sbrk will return cur_brk, whatever the offset
    // heap brk decreases if offset is negative
    // sbrk or brk will not decrease brk below the default brk set by kernel
    //when program is loaded
    void * cur_brk = (void *) syscall_1(SYS_brk,(uint64_t)NULL);
    void * new_brk=(void *)((uint64_t)cur_brk+(uint64_t)offset);
    if(brk(new_brk)==-1){
	//brk() did not set brk pointer to new_brk
	printf("success2\n");
	return (void *)-1;
    }
    else{
	//brk() set the brk pointer to new_brk
	printf("success3\n");
	return cur_brk;
    }
    
}

struct m_blk{
    size_t size;//8bytes
    struct m_blk * next;//8bytes
    size_t free;//8bytes
    char start[1];//1byte
};

typedef struct m_blk * p_blk;

p_blk head=NULL;//head points to the head of the memory linked list
p_blk tail=NULL;//tail points to the last node of the linked list, brk is data chunk + meta

#define M_BLK_SIZE 24 //define size as 24 eventhough 25
//char temp is byte, it will help us do 1 byte pointer arithmetic
//m_blk->temp is the start address of malloc'd memory to be returned

#define ALIGNMENT 8
#define MIN_SPLIT_SIZE 8
#define ALIGN(x) (((x)+(ALIGNMENT-1)) & ~ (ALIGNMENT-1))

p_blk expand_brk(size_t size);
p_blk get_mem_node(size_t size);
int can_split(p_blk t,size_t size);
void split(p_blk t,size_t size);

void *malloc(size_t size){
    p_blk temp;
    size=ALIGN(size);
    if(size==0){
	//refer man page of malloc
	return NULL;
    }
    if(!head){
	//if head is null, malloc has no memory, expand brk
	//set head to meta ptr returned by expand_brk
	//since head is the tail also, tail=head
	temp=expand_brk(size);
	if(temp==NULL){
	    //expand_brk failed, return null
	    return NULL;
	}
	head=temp;
	return head->start;
    }
    else{
	temp=get_mem_node(size);
	if(temp){
	    //temp points to some memory meta node
	    //this chunk has enough space for size
	    if(can_split(temp,size)){
		//if this chunk is too big, split it into two chunks
		//split(temp) splits temp into 2 chunks and uodates the linked list
		split(temp,size);
		
	    }
	    temp->free=0;
	    return temp->start;
	}
	else{
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

}

void free(void *ptr){
    
}


p_blk expand_brk(size_t size){
    //expand_brk raises brk by size+metablock
    //updates tail to new metanode, if head is not null
    //returns tail
    //returns NULL on error
    p_blk temp;
    temp=sbrk(0);
    if(sbrk(size+M_BLK_SIZE)==(void *)-1){
	//sbrk failed
	return NULL;
    }
    if(head){
	//head is not NULL, list already initialized	
	tail->next=temp;
	tail=temp;
	tail->size=size;
	tail->free=0;
	tail->next=NULL;
    }
    else{
	//head is null
	//no memory on heap
	tail=temp;
	tail->size=size;
	tail->free=0;
	tail->next=NULL;
    }
    return temp;

}

p_blk get_mem_node(size_t size){
    p_blk t=head;
    while(t!=NULL){
	if(t->free==1){
	    if( (t->size) >= size ){
		//t size is large enough to fit size
		return t;
	    }
	}
	t=t->next;
    }
    //no free block or no free block big enough
    return NULL;
}

int can_split(p_blk t,size_t size){
    if( (t->size - size) >= (M_BLK_SIZE+MIN_SPLIT_SIZE))
	return 1;
    else
	return 0;
}

void split(p_blk t,size_t size){
    p_blk temp;
    temp=(p_blk)(t->start+size);
    temp->size=t->size-M_BLK_SIZE-size;
    temp->free=1;
    temp->next=t->next;
    t->next=temp;
    t->size=size;
    t->free=0;

    
}
