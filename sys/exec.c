#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/tarfs.h>
#include<sys/memory.h>

uint64_t round_down(uint64_t addr,int n);
void add_tail(struct vma **head,struct vma **tail,struct vma *p);

int exec(char *path,char **argv){

	//char *s,*last;
	uint64_t i;
	char *off;
	uint64_t argc,ustack[4+MAXARG];
	volatile uint64_t sp;
	uint64_t sz;
	Elf64_Ehdr *elf;
	Elf64_Phdr *ph;
	pml4 *pml4_t,*old_pml4_t;
	elf=(Elf64_Ehdr *)tarfs_get_file(path);
	/* assume it's ELF header */
	/* allocate  page table*/
	pml4_t = load_kern_vm();
	if(!pml4_t){
		printf("no space for pml4\n");
		return -1;
	}
	sz=0;
	struct vma *head=NULL;
	struct vma *tail=NULL;
	for(i=0,off=(char *)(elf)+elf->e_phoff;i<elf->e_phnum;i++,off+=sizeof(Elf64_Phdr)){
		ph=(Elf64_Phdr *)off;
		if(ph->p_type!=ELF_PROG_LOAD)
			continue;
		if(ph->p_memsz < ph->p_filesz){ /* ERROR CHECK */
			if(pml4_t){
				free_uvm(pml4_t); /* TODO */
			}
			return -1;
		}
		/* allocuvm allocates physical frames for the p_vaddr->p_vaddr+memsz */
		if((sz=allocuvm(pml4_t,ph->p_vaddr,ph->p_memsz))==0){
			if(pml4_t){
				free_uvm(pml4_t);				
			}
			return -1;
		}
		sz+=ph->p_memsz;
		/* copy bytes from [p_offset,p_offset+p_filesz] to [p_vaddr,p_vaddr+p_filesz] */
		/* load this pml4 into cr3 and then do memcpy. this should work since, virt addr's have been alloc'd. */
		load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
		/* proc page tables already have kernel pagte tables mapped */
		/* do mempcy(des,src,sizse) */
		memcpy((void *)ph->p_vaddr,(void *)(ph->p_offset+(char *)elf),ph->p_filesz);
		/* if filesz < memsz, then remainder is alreadey cleared(0) during inituvm's allocation of pages. */

		/* load pml4 base of kernel page table */
		/* pml4_base is a physical address of 0x100000 */
		load_base((uint64_t)pml4_base); 

		/* vma inits */
		/* create a vma for this program section */
		struct vma *vma_temp=(struct vma *)kmalloc(sizeof(struct vma));
		/* initialze vma.start to p_vaddr virtual addr */
		vma_temp->start=ph->p_vaddr;
		/* initializes vma.end to p_vaddr+p_memsz */
		vma_temp->end=ph->p_vaddr+ph->p_memsz; /* end points 1 byte after the actual end */
		vma_temp->flags=ph->p_flags;		   /* copy flags from elf */
		vma_temp->type=VMA_OTHER;			   /* not stack, not heap */
		vma_temp->next=NULL;
		/* add to vma list */
		add_tail(&head,&tail,vma_temp);
		printf("copying...\n");
		
	}

	/* allocate a frame for stack */
	if((sz=allocuvm(pml4_t,USTACK-0x1000, FRAME_SIZE))==0){
		if(pml4_t){
			free_uvm(pml4_t);				
		}
		return -1;
	}
	sz+=FRAME_SIZE;


	/* sp=USTACK-1;				/\* 1 less than next frame address *\/ */
	sp=USTACK;

	/* push argument strings,prepare rest of stack in ustack */
	for(argc=0;argv[argc];argc++){
		if(argc>=MAXARG){
			if(pml4_t){
				free_uvm(pml4_t);				
			}
			return -1;
		}
		sp=round_down(sp-(strlen(argv[argc])+1),8); /* round down to 8 byte boundary */
		load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
		memcpy((void *)sp,argv[argc],strlen(argv[argc])+1); /* continue here */
		load_base((uint64_t)pml4_base); /* load kernel page tables */
		ustack[3+argc]=sp;
	}
	ustack[3+argc]=0;
	ustack[0]=0xffffffffffffffff; /* fake return PC */
	ustack[1]=argc;
	ustack[2]=sp-(argc+1)*8;	/* argv pointer */

	sp-=(3+argc+1)*8;

	load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
	memcpy((void *)sp,(void *)ustack,(3+argc+1)*8);
	load_base((uint64_t)pml4_base); /* load kernel page tables */
	

	/* save name */
	strcpy(proc->name,path);
	/* commit to the user image */
	old_pml4_t=proc->pml4_t;
	proc->pml4_t=pml4_t;
	proc->size=sz;				/* in bytes */
	proc->tf->rip=elf->e_entry;	/* main */
	proc->tf->rsp=sp;
	switchuvm(proc);
	free_vma_list(proc);		/* free the old vmas */
	proc->vma_head=head;		/* store head of vma's in proc */
	free_uvm(old_pml4_t);			/* TODO */	
	return 0;
}


uint64_t round_down(uint64_t addr,int n){
	if(addr%n==0){
		return addr;
	}
	else{
		return addr - (addr%n);
	}
}

void add_tail(struct vma **head,struct vma **tail,struct vma *p){
	if(*head==NULL){
		/* no element in list */
		*head=p;
		*tail=*head;
	}
	else{
		/* list not empty */
		(*tail)->next=p;
		*tail=p;
	}
}
