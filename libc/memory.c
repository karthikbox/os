#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>
#include <sys/syscall.h>


int brk(void *end_data_segment){
    void * ret = (void *) syscall_1(SYS_brk,(uint64_t)end_data_segment);
    if((uint64_t)ret==(uint64_t)end_data_segment){
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
		return (void *)-1;
    }
    else{
		//brk() set the brk pointer to new_brk
		return cur_brk;
    }
    
}


/* malloc(size_t size) IMPLEMENTATION */
/*

THIS IMPLEMETATION OF MALLOC AND FREE IS AS FOLLOWS:
1. IF NO SPACE, RAISE BRK I.E HEAP BY REQUIRED SIZE
2. IF ANY NON-LAST BLOCK IS FREE'D, IT IS MARKED FREE AND WILL BE ALLOCATED
   TO NEXT REQUEST IF SIZE IS SUFFICIENT. SO DECREASE IN BRK(HEAP)
3. IF LAST NODE IS FREE'D, THEN DECREASE BRK(HEAP)
4. IF ANY INTERMEDIATE IS BEING ALLOCATED TO A REQUEST AND THE BLOCK IS WAY BIGGER THAN REQUESTED SIZE, THEN SPLIT THIS BLK INTO 2 BLOCKS, ALLOCATE THE FIRST ONE TO REQUEST AND THE MARK THE SECOND ONE AS FREE AND ADD IT TO THE LIST.

 */
struct m_blk{
    size_t size;//8bytes
    struct m_blk * next;//8bytes
    struct m_blk * prev;//8bytes
    size_t free;//8bytes
    void * ptr;//8bytes
    char start[1];//1byte
};

typedef struct m_blk * p_blk;

p_blk head=NULL;//head points to the head of the memory linked list
p_blk tail=NULL;//tail points to the last node of the linked list, brk is data chunk + meta

#define M_BLK_SIZE 40 //define size as 24 eventhough 25
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
	temp->prev=tail;
	tail=temp;
	tail->size=size;
	tail->free=0;
	tail->next=NULL;
	tail->ptr=tail->start;
    }
    else{
	//head is null
	//no memory on heap
	tail=temp;
	tail->size=size;
	tail->free=0;
	tail->next=NULL;
	tail->prev=NULL;
	tail->ptr=tail->start;
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
    temp->ptr=temp->start;
    temp->next=t->next;
    temp->prev=t;
	if(t->next!=NULL){
		t->next->prev=temp;		/* if there is a next meta blk, otherwise no point */
	}
    t->next=temp;
    t->size=size;
    t->free=0;

    
}


/* free() IMPLEMENTATION */

int valid_ptr(void *ptr);
p_blk get_meta_ptr(void *ptr);
int remove_last_blk(p_blk t);

void free(void *ptr){
    if(valid_ptr(ptr)){
	p_blk temp=get_meta_ptr(ptr);
	temp->free=1;
	/*	if(temp->next==NULL){
	    //last block is being free'd
	    //decrease the brk pointer to temp
	   
	    //sanity check
	    if(temp!=tail)
		printf("last node to remove is not same as tail\n");

	    tail=temp->prev;
	    if(tail!=NULL){
		tail->next=NULL;
	    }
	    else{
		head=tail;
	    }
	    if(!remove_last_blk(temp))
		printf("unable to remove last blk, just marking it as free\n");;
		}*/
	/*
	  TODO
	  1.implement coalascing
	  2.check prev and next block if they are free then coalasce
	  into 1 big block
	 */
    }
}

int remove_last_blk(p_blk t){
    if(brk((void *)t)==-1){
	printf("unable to decrease brk to remove last blk\n");
	return 0;
    }
    
    return 1;
}

p_blk get_meta_ptr(void *ptr){
    char *temp=ptr;
    temp-=M_BLK_SIZE;
    return (p_blk)temp;
}

int valid_ptr(void *ptr){
    p_blk p=ptr;
    if(head && ptr &&  (p >=head) && (p < (p_blk)sbrk(0)) ) {
	if( p == (get_meta_ptr(ptr))->ptr     ){
	    return 1;
	}
    }
    return 0;
}
