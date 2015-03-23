#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/pmmgr.h>
extern void idt_init();
//extern void pmmgr_init(size_t mem_size,uint64_t* bitmap);

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
	}
	/* Available Physical Memory [0-9fc00]
	   Available Physical Memory [100000-7ffe000] */
	
	/* %p is for uint64_t values -> prints pointer address in hex without overflow */
	/* %x is for 32 bit signed values */
	
	/* physical memory manager */
	
	uint64_t size= (0x7ffe000 - 0x0); //bytes of memory
	uint64_t* map = physfree; 	//assuming free memory from physfree
	pmmgr_init(size,map); //all of memory is set as used
	
	/* now set [0-1MB], [physbase-(physfree-1)], [0x9fc00 - (0x100000-0x1)]as '1'
	set frame corresponding to address physfree as used, since it stores the bitmap
	mem_clear_region(0x100000,(uint64_t)physbase - 0x100000);//[1MB <->( physbase-1)] mark as '0'
	*/
	mem_clear_region(0x100000,(uint64_t)physbase - 0x100000);//[1MB <->( physbase-1)] mark as '0'
	mem_clear_region((uint64_t)physfree+0x2000,0x7ffe000-(uint64_t)physfree-0x2000);//[physfree+2 <-> (END_PHYS_MEM - 1)] mark as '0'. +2 for safety
	
	printf("%p <-> %p\n",physbase,physfree);
	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	clear_line(24);//24 , clears the last line of vga buffer
	idt_init();
	init_timer(100);
	//__asm__ __volatile__("int $0x00");
	//__asm__ __volatile__("int $0x21");
	
	while(1);
	/* blue background and white foreground */
	/* set_color(0x1F); */
	/* clear_screen(); */	
	// kernel starts here
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;

void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	register char *s, *v;
	__asm__(
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);
	reload_gdt();
	setup_tss();
	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	s = "!!!!! start() returned !!!!!";
	for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	while(1);
}
