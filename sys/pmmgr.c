#include<sys/pmmgr.h>
#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/defs.h>

//set bitmap

//size of physical memory in blocks
uint64_t memory_size_in_frames=0;
//number of blocks currently in use
uint64_t memory_used_frames=0;
//memory map bit array
uint64_t* memory_map=0;


void mem_map_set(uint64_t bit){
	memory_map[bit/64] |=(1ul<<(bit%64));
}

int mem_map_clear(uint64_t bit){
	if(mem_map_test(bit) == 0){
		/* if already clear */
		printf("already clear\n");
		/* if it was already clear, return 0 */
		return 0;
	}
	memory_map[bit/64] &= ~(1ul<<(bit%64));
	/* if it was previously set, return 1 */
	return 1;
}

int mem_map_test(uint64_t bit){
	if((memory_map[bit/64] & (1ul<<(bit%64))) > 0)
		return 1;
	else
		return 0;
}

long mem_map_first_free(size_t size){
	int number_of_frames=NUMBER_OF_FRAMES(size);
	/* sanity check for number_of_frames */
	if(number_of_frames > 64){
		printf("sanity check for number_of_frames - %d failed..\n",number_of_frames);
		return -1;
	}
	uint64_t temp=0xffffffffffffffff;
	temp=temp>>(64-number_of_frames);
	for(uint64_t i=0;i<memory_size_in_frames/64+1;i++){
		if(memory_map[i]!=0xffffffffffffffff){
			for(int j=0;j<64;j++){
				if( !(memory_map[i] & (temp<<(j))) ){
					return j+i*64;//return bit number
				}
			}
		}
	}
	return -1;
}

void mem_clear_region(uint64_t base,size_t size){
	uint64_t align = base/FRAME_SIZE;
	uint64_t blocks= size/FRAME_SIZE;
	for(;blocks>0;blocks--){
		mem_map_clear(align++);
		memory_used_frames--;
	}
	mem_map_set(0);
}

void mem_set_region(uint64_t base,size_t size){
	uint64_t align = base/FRAME_SIZE;
	uint64_t blocks= size/FRAME_SIZE;
	for(;blocks>0;blocks--){
		mem_map_clear(align++);
		memory_used_frames++;
	}

}




uint64_t get_memory_frame_count(){
	//returns number of frames which can hold the physical memory

	uint64_t n=memory_size/FRAME_SIZE;
	if((memory_size%FRAME_SIZE)!=0){
		return n+1;
	}
	else
		return n;
}

//size_t is same as uint64_t
void pmmgr_init(size_t mem_size,uint64_t* bitmap){
	//physical memory must be a multiple of 4kb
	// or else last frame will only be partially have physical memory
	// this could be a problem
	// no problem, if mem_size is a multiple of 4KB
	//assuming mem_size if multiple of 4KB
	memory_size=mem_size;//mem_size in bytes
	memory_map=bitmap;
	memory_size_in_frames=get_memory_frame_count();
	memory_used_frames=get_memory_frame_count();/* mark all frames as used */

	memset1((char *)memory_map,0xFF,memory_size_in_frames/FRAMES_PER_BYTE);
	memset1((char *)memory_map+memory_size_in_frames/FRAMES_PER_BYTE,0xFF,1);
	
}

long get_free_frame_count(){
	return (memory_size_in_frames - memory_used_frames);
}


void* alloc_frame(size_t size){
	int i=0;
	if(get_free_frame_count() <= 0){
		return 0;
	}
	//there is free memory
	long frame_number=mem_map_first_free(size);
	if(frame_number==-1)
		return 0;
	int num=NUMBER_OF_FRAMES(size);
	for(i=0;i<num;i++){
		mem_map_set((uint64_t)frame_number+i);
		memory_used_frames++;
	}
	uint64_t addr=frame_number*FRAME_SIZE;
	return (void *)addr;

}


void free_frame(void *a){
	/* a is phys addr */
	uint64_t addr=(uint64_t)a;
	uint64_t bit = addr/FRAME_SIZE;
	if(bit > (memory_size_in_frames + 2) ){
		printf("kfree issued on a non existent physical frame\n");
		return ;
	}
		
	if(a){
		if(mem_map_clear(bit)==1){
			/* if it was previously set and now it is clear */
			memory_used_frames--;
		}
		/* if it was preciously clear, then dont decr count */
	}	

}

int init_ref_map(struct ref_map *p){
	/* return 0 on failure, 1 on success */
	/* allocate 1 byte for every frame */
	/* number of frames is get_memory_frame_count */
	p->entries=(int *)kmalloc(get_memory_frame_count()*sizeof(int));
	if(!p->entries){
		return 0;
	}
	/* set all ref counts to 0 */
	memset1((char *)p->entries,0,get_memory_frame_count()*sizeof(int));	
	/* set ref_count,global var as p->entries */
	ref_count=p->entries;
	return 1;
	
}


int get_ref_count(uint64_t phys_addr){
	/* return current ref value of frame */
	/* takes in phys addr of frame */

	/* get frame number */
	uint64_t bit = phys_addr/FRAME_SIZE;
	/* return ref count of frame number */
	return ref_count[bit];
}

void set_ref_count(uint64_t phys_addr,int val){
	/* takes in phys address and value of ref_count */
	uint64_t bit=phys_addr/FRAME_SIZE;
	ref_count[bit]=val;
}

void incr_ref_count(uint64_t phys_addr){
	uint64_t bit=phys_addr/FRAME_SIZE;
	ref_count[bit]+=1;
}

void decr_ref_count(uint64_t phys_addr){
	uint64_t bit=phys_addr/FRAME_SIZE;
	ref_count[bit]-=1;	
	
}
