#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/pmmgr.h>
#include <sys/page.h>
#include <sys/process.h>
#include<sys/tarfs.h>
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
	uint64_t* map = (uint64_t *)get_virt_addr((uint64_t)physfree); 	//assuming free memory from physfree
	pmmgr_init(size,map); //all of memory is set as used
	
	/* now set [0-1MB], [physbase-(physfree-1)], [0x9fc00 - (0x100000-0x1)]as '1'
	set frame corresponding to address physfree as used, since it stores the bitmap
	mem_clear_region(0x100000,(uint64_t)physbase - 0x100000);//[1MB <->( physbase-1)] mark as '0'
	*/
	mem_clear_region(0x100000,(uint64_t)physbase - 0x100000);//[1MB <->( physbase-1)] mark as '0'
	mem_clear_region((uint64_t)physfree+0x2000,0x7ffe000-(uint64_t)physfree-0x2000);//[physfree+2 <-> (END_PHYS_MEM - 1)] mark as '0'. +2 for safety
	
	/* vm manager */
	vm_init(physbase,physfree);
	//*((char *)0xffffffff80000000ul+(uint64_t)physfree)='a';
	//*((char *)(0xffffffff80000000ul+0x200000ul))='a';
	printf("%p\n",(uint64_t)alloc_frame(PAGE_SIZE));
	printf("%p <-> %p\n",physbase,physfree);
	printf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	clear_line(24);//24 , clears the last line of vga buffer
	idt_init();
	init_timer(100);
        
	int* one=(int*)kmalloc(4096*2);
	printf("one: %p",one);
	int* two=(int*)kmalloc(8);
	printf("two: %p",two);
	int* three=(int*)kmalloc(4096);
	printf("three: %p",three);
	int* four=(int*)kmalloc(12);
	printf("four:%p",four);
	kfree(one);
	one=(int*)kmalloc(40);
	printf("one again: %p",one);
	int* five=(int*)kmalloc(4096);
	printf("five:last+1000 %p",five);
	int* num;
	for(int i=0; i<105; i++)
	{
		num = (int*)kmalloc(sizeof(int));
	}
	printf("num is %p\n", num);
	//__asm__ __volatile__("int $0x00");
	//__asm__ __volatile__("int $0x80");
	uint64_t *elf_start;
	if(!(elf_start=tarfs_get_file("bin/initcode"))){
		printf("elf not found\n");
	}
	printf("found at %p\n",elf_start);
	printf("pml4_base->%p\n",(uint64_t)pml4_base);
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
