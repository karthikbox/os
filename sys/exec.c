#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/tarfs.h>
#include<sys/kmalloc.h>


int exec(char *path,char **argv){

	//char *s,*last;
	uint64_t i;
	char *off;
	//uint64_t argc,sz,sp,ustack[4+MAXARG];
	uint64_t sz;
	Elf64_Ehdr *elf;
	Elf64_Phdr *ph;
	pml4 *pml4_t/* ,*old_pml4_t */;
	elf=(Elf64_Ehdr *)tarfs_get_file(path);
	/* assume it's ELF header */
	/* allocate  page table*/
	pml4_t = load_kern_vm();
	if(!pml4_t){
		printf("no space for pml4\n");
		return -1;
	}
	sz=0;
	for(i=0,off=(char *)(elf)+elf->e_phoff;i<elf->e_phnum;i++,off+=sizeof(Elf64_Phdr)){
		ph=(Elf64_Phdr *)off;
		if(ph->p_type!=ELF_PROG_LOAD)
			continue;
		if(ph->p_memsz < ph->p_filesz){
			if(pml4_t){
				kfree(pml4_t);				
			}
			return -1;
		}
		/* allocuvm allocates physical frames for the p_vaddr->p_vaddr+memsz */
		if((sz=allocuvm(pml4_t,ph->p_vaddr,ph->p_memsz))==0){
			if(pml4_t){
				kfree(pml4_t);				
			}
			return -1;
		}
		/* copy bytes from [p_offset,p_offset+p_filesz] to [p_vaddr,p_vaddr+p_filesz] */
		/* load this pml4 into cr3 and then do memcpy. this should work since, virt addr's have been alloc'd. */
		load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
		/* proc page tables already have kernel pagte tables mapped */
		/* do mempcy(des,src,sizse) */
		memcpy((void *)ph->p_vaddr,(void *)(ph->p_offset+(uint64_t)elf),ph->p_filesz);
		/* if filesz < memsz, then remainder is alreadey cleared(0) during inituvm's allocation of pages. */
		//load_base(())
		
		/* if(loaduvm(pml4_t,(char *)ph->p_vaddr,ip,ph->p_offset,ph->p_filesz)<0){ */
		/* 	if(pml4_t){ */
		/* 		kfree(pml4_t); */
		/* 	} */
		/* 	return -1; */
		/* } */
		
		/*  */
	}
	return 0;
}
