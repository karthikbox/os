#include<sys/pmmgr.h>
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

void mem_map_clear(uint64_t bit){
	memory_map[bit/64] &= ~(1ul<<(bit%64));
}

int mem_map_test(uint64_t bit){
	if((memory_map[bit/64] & (1ul<<(bit%64))) > 0)
		return 1;
	else
		return 0;
}

long mem_map_first_free(size_t size){
	int number_of_frames=NUMBER_OF_FRAMES(size);
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
	uint64_t addr=(uint64_t)a;
	uint64_t bit = addr/FRAME_SIZE;
	if(a){
		mem_map_clear(bit);
		memory_used_frames--;
	}	

}
