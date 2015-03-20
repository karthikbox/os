#include<sys/pmmgr.h>
#include<sys/utility.h>
#include<sys/defs.h>
//set bitmap
void mem_map_set(uint64_t bit){
	memory_map[bit/64] |=(1<<(bit%64));
}

void mem_map_clear(uint64_t bit){
	memory_map[bit/64] &= ~(1<<(bit%64));
}

int mem_map_test(uint64_t bit){
	if((memory_map[bit/64] & (1<<(bit%64))) > 0)
		return 1;
	else
		return 0;
}

long mem_map_first_free(){
	for(uint64_t i=0;i<memory_size_in_frames/64;i++){
		if(memory_map[i]!=0xffffffffffffffff){
			for(int j=0;j<64;j++){
				if( !(memory_map[i] & (1<<(j)) ) ){
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
	
}

long get_free_frame_count(){
	return (memory_size_in_frames - memory_used_frames);
}


void* alloc_frame(){
	if(get_free_frame_count() <= 0){
		return 0;
	}
	//there is free memory
	long frame_number=mem_map_first_free();
	if(frame_number==-1)
		return 0;
	mem_map_set((uint64_t)frame_number);
	memory_used_frames++;
	uint64_t addr=frame_number*FRAME_SIZE;
	return (void *)addr;

}


void free_frame(void *a){
	uint64_t addr=(uint64_t)a;
	uint64_t bit = addr/FRAME_SIZE;
	mem_map_clear(bit);
	memory_used_frames--;

}
