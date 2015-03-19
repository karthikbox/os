#ifndef __PMMGR_H
#define __PMMGR_H

#define BLOCKS_PER_BYTE 8
#define BLOCK_SIZE 4096
#define BLOCK_ALIGN BLOCK_SIZE


//size of physical memory
uint64_t memory_size=0;//uint64_t is same as size_t
//number of blocks currently in use
uint64_t used_blocks=0;
//memory map bit array
uint64_t* memory_map=0;


//set bitmap
void mem_map_set(uint64_t bit){
	memory_map[bit/64] |=(1<<(bit%64));
}

void mem_map_clear(uint64_t bit){
	memory_map[bit/64] &= ~(1<<(bit%64));
}

int mem_map_test(uint64_t bit){
	if(memory_map[bit/64] & (1<<(bit%64)) > 0)
		return 1;
	else
		return 0;
}

int mem_map_first_free(){
	for(uint64_t i=0;i<memory_size/64;i++){
		if(memory_map[i]!=0xffffffffffffffff){
			for(int j=0;j<64;j++){
				if( !(memory_map[i] & (1<<(j)) ) ){
					return j+i*64;//return bit number
				}
			}
		}
	}
}

//size_t is same as uint64_t
void pmmgr_init(size_t mem_size,physical_addr bitmap){
	memory_size=mem_size;
	memory_map=(uint64_t *)bitmap;
	memory_
}
#endif
