#ifndef __PMMGR_H
#define __PMMGR_H

#include<sys/defs.h>

#define FRAMES_PER_BYTE 8
#define FRAME_SIZE 4096
#define FRAME_ALIGN FRAME_SIZE


//size of physical memory
uint64_t memory_size;//uint64_t is same as size_t

//size of physical memory in blocks
uint64_t memory_size_in_frames;
//number of blocks currently in use
uint64_t memory_used_frames;
//memory map bit array
uint64_t* memory_map;


//set bitmap
void mem_map_set(uint64_t bit);

void mem_map_clear(uint64_t bit);


int mem_map_test(uint64_t bit);

long mem_map_first_free();

//size_t is same as uint64_t
void pmmgr_init(size_t mem_size,uint64_t* bitmap);

void mem_clear_region(uint64_t base,size_t size);

void mem_set_region(uint64_t base,size_t size);

uint64_t get_memory_frame_count();

long get_free_frame_count();

void* alloc_frame();

void free_frame(void *a);

#endif
