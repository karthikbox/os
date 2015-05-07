#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/process.h>
#include<sys/page.h>
#include<sys/pmmgr.h>
#include<sys/tarfs.h>
#include<sys/memory.h>
#include<errno.h>
uint64_t round_down(uint64_t addr,int n);
void clear_gpr(struct trapframe *s);
int exec_new(char *path,char **argv,char **envp);


int exec(char *path,char **argv,char **envp){
	/* path, argv,envp address range checking done in syscall.c */
	/* path is being used below */
	//char *s,*last;
	uint64_t i;
	char *off;
	uint64_t argc,ustack[4+MAXARG];
	volatile uint64_t sp;
	uint64_t sz;
	Elf64_Ehdr *elf;
	Elf64_Phdr *ph;
	pml4 *pml4_t,*old_pml4_t;
	/* copy path to kernel memory */
	char *kpath = (char*)kmalloc(NCHARS*sizeof(char));
	if(kpath==NULL){
		return -ENOMEM;			/* NO KERNEL MEMORY */
	}
		
	strcpy(kpath, (char *)path);

	if(get_absolute_path(kpath) == NULL){
		kfree(kpath);
		return -ENAMETOOLONG;
	}
	/* files don't have trailing slash */
	kpath[strlen(kpath)-1]='\0';
	char *file=(char *)tarfs_get_file(kpath,REGTYPE);
	elf=(Elf64_Ehdr *)file;
	kfree(kpath);
	/* if elf is null, then no file exists with that name */
	if(elf==NULL)
		return -ENOENT;			/* NO FILE FOUND */
	
	/*  if file satrts with #! */
	if((file[0]=='#') && (file[1]=='!')){
		/* it is a script file */
		/* get first line from file */
		int k=0;
		char *str=(char *)kmalloc(0x1000);
		memset1((char *)str,0,0x1000);
		int s=0;
		for(;file[k]!='\n';k++){
			/* while cur char is not \n */
			str[k]=file[k];
			s++;
		}
		/* now str has first line */
		str[++s]='\0';
		int token_len;
		/* tokenize str on space */
		char **tokens=strtoken(str," ",&token_len);
		/* first token has #! */
		if(strcmp(tokens[0],"#!")==0){
			printf("check pass\n");
		}
		/* second token has ELF path */

		
		/* offset argv elements by 1 */
		int arg=0;
		k=0;
		while(argv[arg]){
			arg++;
		}
		arg++;						/* for NULL byte of old argv */
		arg++;						/* for new filename */
		arg++;						/* for interpreter path */
		/* format of argv to interpreter  */
		/* argv[0]->interpreter path */
		/* argv[1]->script filepath */
		/* argv[2]->original argv[0] */
		if(arg >= MAXARG ){
			free_array(tokens,token_len);
			kfree(str);
			return -E2BIG;	/* MORE THAN MAX NUMBER OF ARGUMENTS */
		}
		/* alloc new argv, which has script path+old argv */
		char **kargv=(char **)kmalloc(arg*sizeof(char *));	
		if(kargv==NULL){
			free_array(tokens,token_len);
			kfree(str);
			return -ENOMEM;
		}
		/* clear new argc */
		memset1((char *)kargv,0,8*arg);
		
		for(k=0;k<(arg-1);k++){
			/* allocate only for interpreter path and script filename */
			/* remaining will use previous argv values */
			if(k==0){
				/* tokens[1] has interpreter path */
				char *mem=(char *)kmalloc(sizeof(char *)*(strlen(tokens[1])+1));
				if(mem==NULL){
					free_array(tokens,token_len);
					kfree(str);
					kfree(kargv);
					return -ENOMEM;
				}
				strcpy(mem,tokens[1]);
				kargv[k]=mem;
			}
			else if(k==1){
				/* path contains script file path */
				char *mem=(char *)kmalloc(sizeof(char)*(strlen(path)+1));
				if(mem==NULL){
					free_array(tokens,token_len);
					kfree(str);
					kfree(kargv);
					return -ENOMEM;
				}
				strcpy(mem,path);
				kargv[k]=mem;
			}
			else{
				/* char *mem=(char *)kmalloc(sizeof(char)*(strlen(argv[k])+1)); */
				/* strcpy(mem,path); */
				kargv[k]=argv[k-1];
			}
		}
		kargv[k]=NULL;

		int ret=exec_new(tokens[1],kargv,envp);

	
		kfree(kargv[0]);			/* free only the script filepath */
		kfree(kargv);				/* free only the new array */
		free_array(tokens,token_len);
		kfree(str);
		return ret;
		/* then get path */
	}
	/* else if does is it start with ELF magic number */
	else if((elf->e_ident[0] == 0x7f ) && (elf->e_ident[1] == 'E' ) && (elf->e_ident[2] == 'L' ) && (elf->e_ident[3] == 'F' )){
		/* assume it's ELF header */
		/* allocate  page table*/
		pml4_t = load_kern_vm();
		if(!pml4_t){
			printf("no space for pml4\n");
			return -ENOMEM;				/* NO KERN MEMORY */
		}
		sz=0;
		struct vma *head=NULL;
		struct vma *tail=NULL;
		uint64_t last_seg_start=0;
		uint64_t last_seg_size=0;
		for(i=0,off=(char *)(elf)+elf->e_phoff;i<elf->e_phnum;i++,off+=sizeof(Elf64_Phdr)){
			ph=(Elf64_Phdr *)off;
			if(ph->p_type!=ELF_PROG_LOAD)
				continue;
			if(ph->p_memsz < ph->p_filesz){ /* ERROR CHECK */
				if(pml4_t){
					free_uvm(pml4_t);
				}
				free_vma_list(&head); /* free new vma */
				return -ENOEXEC;	  /* BAD FORMAT */
			}
			/* get the elf flags for this segment */
			/* ignore elf's read,execute flags cuz no meaning in page tables */
			/* ph->p_flags */
			/* convert elf p_header flags to page table entries flags */
			uint64_t flags=0;
			if(ph->p_flags & PF_W){
				flags=flags | PT_WRITABLE; /* set writiable bit in pt entries */
			}
			/* since these pages are for user, set user bit 1 in page table */
			flags=flags | PT_USER;
			/* allocuvm allocates physical frames for the p_vaddr->p_vaddr+memsz */
			if((sz=allocuvm(pml4_t,ph->p_vaddr,ph->p_memsz,flags))==0){
				if(pml4_t){
					free_uvm(pml4_t);				
				}
				free_vma_list(&head); /* free new vma */
				return -ENOMEM;			  /* NO KERN MEMORY */
			}
			sz+=ph->p_memsz;
			/* copy bytes from [p_offset,p_offset+p_filesz] to [p_vaddr,p_vaddr+p_filesz] */
			/* load this pml4 into cr3 and then do memcpy. this should work since, virt addr's have been alloc'd. */
			load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
			/* proc page tables already have kernel pagte tables mapped */
			/* do mempcy(des,src,sizse) */
			/* disable wp bit of kernel */
			/* so that we can copy contents to rad only regions */
			clear_wp_bit();
			memcpy((void *)ph->p_vaddr,(void *)(ph->p_offset+(char *)elf),ph->p_filesz);
			/* if filesz < memsz, then remainder is alreadey cleared(0) during inituvm's allocation of pages. */
			
			/* load pml4 base of kernel page table */
			/* pml4_base is a physical address of 0x100000 */
			load_base((uint64_t)pml4_base); 
			/* enable wp bit pf kernel, needed got kernel COW */
			set_wp_bit();
			/* vma inits */
			/* create a vma for this program section */
			struct vma *vma_temp=(struct vma *)kmalloc(sizeof(struct vma));
			if(!vma_temp){
				if(pml4_t){
					free_uvm(pml4_t);				
			}
				free_vma_list(&head); /* free new vma */
				return -ENOMEM;		  /* NO KERN MEMORY */
			}
			
			/* initialze vma.start to p_vaddr virtual addr */
			vma_temp->start=ph->p_vaddr;
			/* initializes vma.end to p_vaddr+p_memsz */
			vma_temp->end=ph->p_vaddr+ph->p_memsz; /* end points 1 byte after the actual end */
			vma_temp->flags=ph->p_flags;		   /* copy flags from elf */
			vma_temp->next=NULL;
			/* add to vma list */
			add_tail(&head,&tail,vma_temp);
			/* printf("vma.start-> %p ## vma.end-> %p ## vma.flags-> %x\n",vma_temp->start,vma_temp->end,vma_temp->flags);		 */
			/* get max of all section program header star addrs */
			if(last_seg_start < ph->p_vaddr){
				last_seg_start=ph->p_vaddr;
				last_seg_size=ph->p_memsz;
			}
		}
		
		/* allocate a frame for stack */
		/* stack pt_flags will have flags WRITABLE,USER */
		if((sz=allocuvm(pml4_t,USTACK-0x1000, FRAME_SIZE,(PT_WRITABLE|PT_USER)))==0){
			if(pml4_t){
				free_uvm(pml4_t);				
			}
			free_vma_list(&head); /* free new vma */
			return -ENOMEM;		  /* NO KERN MEMORY */
		}
		sz+=FRAME_SIZE;
		
		
		/* sp=USTACK-1;				/\* 1 less than next frame address *\/ */
		sp=USTACK;
		
		/* vma for stack */
		/* allocate */
		struct vma *vma_stk=(struct vma *)kmalloc(sizeof(struct vma));
		if(!vma_stk){
			if(pml4_t){
				free_uvm(pml4_t);				
			}
			free_vma_list(&head); /* free new vma */
			return -ENOMEM;		  /* NO KERN MEMORY */
		}
		/* initiaize */
		/* vma.start= USTACK-0x1000 */
		vma_stk->start=USTACK-0x1000;
		/* vma.end= USTACK */
		vma_stk->end=USTACK;
		/* vma.type= VMA_STACK */
		/* vma_stk->type=VMA_STACK; */
		/* vma.flags=??? */
		vma_stk->flags=PF_R | PF_W |PF_X|PF_GROWSDOWN; /* stack has read, write, exec, growsdown flags set */
		/* add stack vma to new vma_list */
		vma_stk->next=NULL;
		add_tail(&head,&tail,vma_stk);
		
		/* TODO: heap vma */
		/* HEAP PT_FLAGS WILL HAVE PT_WRITABLE|PT_USER */
		struct vma *vma_heap=(struct vma *)kmalloc(sizeof(struct vma));
		if(!vma_heap){
			if(pml4_t){
				free_uvm(pml4_t);				
		}
			free_vma_list(&head); /* free new vma */
			return -ENOMEM;
		}
		/* initializee */
		/* initially heap has no memory. so vma.start and end point to same address */
		/* (last_seg_start + last_seg_size ) is the edn of the highest section*/
		/* start heap from end of last section + 0x1000 */
		/* TODO; round up page to neaserst frame boundary */
		vma_heap->start=(last_seg_start+last_seg_size+0x3000);
		/* heap initially has 1 byte */
		vma_heap->end=vma_heap->start+0x1ul;
		/* heap has read, write, exec, growsup flags set */
		vma_heap->flags=PF_R | PF_W |PF_X|PF_GROWSUP;
		vma_heap->next=NULL;
		add_tail(&head,&tail,vma_heap);
		
		/* push argument strings,prepare rest of stack in ustack */
		for(argc=0;argv[argc];argc++){
			if(argc>=MAXARG){
				if(pml4_t){
					free_uvm(pml4_t);
				}
				free_vma_list(&head); /* free new vma */
				return -E2BIG;
			}
			sp=round_down(sp-(strlen(argv[argc])+1),8); /* round down to 8 byte boundary */
			load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
			memcpy((void *)sp,argv[argc],strlen(argv[argc])+1); /* continue here */
			load_base((uint64_t)pml4_base); /* load kernel page tables */
			ustack[2+argc]=sp;
		}
		ustack[2+argc]=0;
		ustack[0]=0xffffffffffffffff; /* fake return PC */
		ustack[1]=argc;
		argc++;
		int envp_c;
		/* for envp */
		for(envp_c=0;envp[envp_c];argc++,envp_c++){
			if(envp_c>=MAXARG){
				if(pml4_t){
					free_uvm(pml4_t);
				}
				free_vma_list(&head); /* free new vma */
			return -E2BIG;
			}
			sp=round_down(sp-(strlen(envp[envp_c])+1),8); /* round down to 8 byte boundary */
			load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
			memcpy((void *)sp,envp[envp_c],strlen(envp[envp_c])+1); /* continue here */
			load_base((uint64_t)pml4_base); /* load kernel page tables */
			ustack[2+argc]=sp;
		}
		ustack[2+argc]=0;
		/* ustack[2]=sp-(argc+1)*8;	/\* argv pointer *\/ */
		sp-=(3+argc)*8;
		
		load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
		memcpy((void *)sp,(void *)ustack,(3+argc)*8);
		load_base((uint64_t)pml4_base); /* load kernel page tables */
		
		
		/* save name */
		strcpy(proc->name,path);
		/* commit to the user image */
		old_pml4_t=proc->pml4_t;
		proc->pml4_t=pml4_t;
		proc->size=sz;				/* in bytes */
		proc->tf->rip=elf->e_entry;	/* main */
		proc->tf->rsp=sp;
		clear_gpr(proc->tf);		/* clear only the gprs of trapframe */
		switchuvm(proc);
		free_vma_list(&proc->vma_head);		/* free the old vmas */
		proc->vma_head=head;		/* store head of vma's in proc */
		free_uvm(old_pml4_t);
		return 0;
	}
	/* else return -ENOEXEC */
	else{
		return -ENOEXEC;
	}

}


void clear_gpr(struct trapframe *s){
	s->rbp=0x0ul;
	s->r15=0x0ul;
	s->r14=0x0ul;
	s->r13=0x0ul;
	s->r12=0x0ul;
	s->r11=0x0ul;
	s->r10=0x0ul;
	s->r9=0x0ul;
	s->r8=0x0ul;
	s->rdi=0x0ul;
	s->rsi=0x0ul;
	s->rdx=0x0ul;
	s->rcx=0x0ul;
	s->rbx=0x0ul;
	s->rax=0x0ul;
	
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


int exec_new(char *path,char **argv,char **envp){
	/* path is the interpreter path */

	uint64_t i;
	char *off;
	uint64_t argc,ustack[4+MAXARG];
	volatile uint64_t sp;
	uint64_t sz;
	Elf64_Ehdr *elf;
	Elf64_Phdr *ph;
	pml4 *pml4_t,*old_pml4_t;
	/* copy path to kernel memory */
	char *kpath = (char*)kmalloc(NCHARS*sizeof(char));
	if(kpath==NULL){
		return -ENOMEM;			/* NO KERNEL MEMORY */
	}
		
	strcpy(kpath, path);	/* tokens[1] contains elf interpreter path */

	if(get_absolute_path(kpath) == NULL){
		kfree(kpath);
		return -ENAMETOOLONG;
	}
	/* files don't have trailing slash */
	kpath[strlen(kpath)-1]='\0';
	char *file=(char *)tarfs_get_file(kpath,REGTYPE);
	elf=(Elf64_Ehdr *)file;
	kfree(kpath);
	/* if elf is null, then no file exists with that name */
	if(elf==NULL)
		return -ENOENT;			/* NO FILE FOUND */
	
	/*  if file satrts with #! */
	if((file[0]=='#') && (file[1]=='!')){
		return -EACCES;
	}
	/* else if does is it start with ELF magic number */
	else if((elf->e_ident[0] == 0x7f ) && (elf->e_ident[1] == 'E' ) && (elf->e_ident[2] == 'L' ) && (elf->e_ident[3] == 'F' )){
		/* assume it's ELF header */
		/* allocate  page table*/
		pml4_t = load_kern_vm();
		if(!pml4_t){
			printf("no space for pml4\n");
			return -ENOMEM;				/* NO KERN MEMORY */
		}
		sz=0;
		struct vma *head=NULL;
		struct vma *tail=NULL;
		uint64_t last_seg_start=0;
		uint64_t last_seg_size=0;
		for(i=0,off=(char *)(elf)+elf->e_phoff;i<elf->e_phnum;i++,off+=sizeof(Elf64_Phdr)){
			ph=(Elf64_Phdr *)off;
			if(ph->p_type!=ELF_PROG_LOAD)
				continue;
			if(ph->p_memsz < ph->p_filesz){ /* ERROR CHECK */
				if(pml4_t){
					free_uvm(pml4_t);
				}
				free_vma_list(&head); /* free new vma */
				return -ENOEXEC;	  /* BAD FORMAT */
			}
			/* get the elf flags for this segment */
			/* ignore elf's read,execute flags cuz no meaning in page tables */
			/* ph->p_flags */
			/* convert elf p_header flags to page table entries flags */
			uint64_t flags=0;
			if(ph->p_flags & PF_W){
				flags=flags | PT_WRITABLE; /* set writiable bit in pt entries */
			}
			/* since these pages are for user, set user bit 1 in page table */
			flags=flags | PT_USER;
			/* allocuvm allocates physical frames for the p_vaddr->p_vaddr+memsz */
			if((sz=allocuvm(pml4_t,ph->p_vaddr,ph->p_memsz,flags))==0){
				if(pml4_t){
					free_uvm(pml4_t);				
				}
				free_vma_list(&head); /* free new vma */
				return -ENOMEM;			  /* NO KERN MEMORY */
			}
			sz+=ph->p_memsz;
			/* copy bytes from [p_offset,p_offset+p_filesz] to [p_vaddr,p_vaddr+p_filesz] */
			/* load this pml4 into cr3 and then do memcpy. this should work since, virt addr's have been alloc'd. */
			load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
			/* proc page tables already have kernel pagte tables mapped */
			/* do mempcy(des,src,sizse) */
			/* disable wp bit of kernel */
			/* so that we can copy contents to rad only regions */
			clear_wp_bit();
			memcpy((void *)ph->p_vaddr,(void *)(ph->p_offset+(char *)elf),ph->p_filesz);
			/* if filesz < memsz, then remainder is alreadey cleared(0) during inituvm's allocation of pages. */
			
			/* load pml4 base of kernel page table */
			/* pml4_base is a physical address of 0x100000 */
			load_base((uint64_t)pml4_base); 
			/* enable wp bit pf kernel, needed got kernel COW */
			set_wp_bit();
			/* vma inits */
			/* create a vma for this program section */
			struct vma *vma_temp=(struct vma *)kmalloc(sizeof(struct vma));
			if(!vma_temp){
				if(pml4_t){
					free_uvm(pml4_t);				
			}
				free_vma_list(&head); /* free new vma */
				return -ENOMEM;		  /* NO KERN MEMORY */
			}
			
			/* initialze vma.start to p_vaddr virtual addr */
			vma_temp->start=ph->p_vaddr;
			/* initializes vma.end to p_vaddr+p_memsz */
			vma_temp->end=ph->p_vaddr+ph->p_memsz; /* end points 1 byte after the actual end */
			vma_temp->flags=ph->p_flags;		   /* copy flags from elf */
			vma_temp->next=NULL;
			/* add to vma list */
			add_tail(&head,&tail,vma_temp);
			/* printf("vma.start-> %p ## vma.end-> %p ## vma.flags-> %x\n",vma_temp->start,vma_temp->end,vma_temp->flags);		 */
			/* get max of all section program header star addrs */
			if(last_seg_start < ph->p_vaddr){
				last_seg_start=ph->p_vaddr;
				last_seg_size=ph->p_memsz;
			}
		}
		
		/* allocate a frame for stack */
		/* stack pt_flags will have flags WRITABLE,USER */
		if((sz=allocuvm(pml4_t,USTACK-0x1000, FRAME_SIZE,(PT_WRITABLE|PT_USER)))==0){
			if(pml4_t){
				free_uvm(pml4_t);				
			}
			free_vma_list(&head); /* free new vma */
			return -ENOMEM;		  /* NO KERN MEMORY */
		}
		sz+=FRAME_SIZE;
		
		
		/* sp=USTACK-1;				/\* 1 less than next frame address *\/ */
		sp=USTACK;
		
		/* vma for stack */
		/* allocate */
		struct vma *vma_stk=(struct vma *)kmalloc(sizeof(struct vma));
		if(!vma_stk){
			if(pml4_t){
				free_uvm(pml4_t);				
			}
			free_vma_list(&head); /* free new vma */
			return -ENOMEM;		  /* NO KERN MEMORY */
		}
		/* initiaize */
		/* vma.start= USTACK-0x1000 */
		vma_stk->start=USTACK-0x1000;
		/* vma.end= USTACK */
		vma_stk->end=USTACK;
		/* vma.type= VMA_STACK */
		/* vma_stk->type=VMA_STACK; */
		/* vma.flags=??? */
		vma_stk->flags=PF_R | PF_W |PF_X|PF_GROWSDOWN; /* stack has read, write, exec, growsdown flags set */
		/* add stack vma to new vma_list */
		vma_stk->next=NULL;
		add_tail(&head,&tail,vma_stk);
		
		/* TODO: heap vma */
		/* HEAP PT_FLAGS WILL HAVE PT_WRITABLE|PT_USER */
		struct vma *vma_heap=(struct vma *)kmalloc(sizeof(struct vma));
		if(!vma_heap){
			if(pml4_t){
				free_uvm(pml4_t);				
		}
			free_vma_list(&head); /* free new vma */
			return -ENOMEM;
		}
		/* initializee */
		/* initially heap has no memory. so vma.start and end point to same address */
		/* (last_seg_start + last_seg_size ) is the edn of the highest section*/
		/* start heap from end of last section + 0x1000 */
		/* TODO; round up page to neaserst frame boundary */
		vma_heap->start=(last_seg_start+last_seg_size+0x3000);
		/* heap initially has 1 byte */
		vma_heap->end=vma_heap->start+0x1ul;
		/* heap has read, write, exec, growsup flags set */
		vma_heap->flags=PF_R | PF_W |PF_X|PF_GROWSUP;
		vma_heap->next=NULL;
		add_tail(&head,&tail,vma_heap);
		
		/* push argument strings,prepare rest of stack in ustack */
		for(argc=0;argv[argc];argc++){
			if(argc>=MAXARG){
				if(pml4_t){
					free_uvm(pml4_t);
				}
				free_vma_list(&head); /* free new vma */
				return -E2BIG;
			}
			sp=round_down(sp-(strlen(argv[argc])+1),8); /* round down to 8 byte boundary */
			load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
			memcpy((void *)sp,argv[argc],strlen(argv[argc])+1); /* continue here */
			load_base((uint64_t)pml4_base); /* load kernel page tables */
			ustack[2+argc]=sp;
		}
		ustack[2+argc]=0;
		ustack[0]=0xffffffffffffffff; /* fake return PC */
		ustack[1]=argc;
		argc++;
		int envp_c;
		/* for envp */
		for(envp_c=0;envp[envp_c];argc++,envp_c++){
			if(envp_c>=MAXARG){
				if(pml4_t){
					free_uvm(pml4_t);
				}
				free_vma_list(&head); /* free new vma */
			return -E2BIG;
			}
			sp=round_down(sp-(strlen(envp[envp_c])+1),8); /* round down to 8 byte boundary */
			load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
			memcpy((void *)sp,envp[envp_c],strlen(envp[envp_c])+1); /* continue here */
			load_base((uint64_t)pml4_base); /* load kernel page tables */
			ustack[2+argc]=sp;
		}
		ustack[2+argc]=0;
		/* ustack[2]=sp-(argc+1)*8;	/\* argv pointer *\/ */
		sp-=(3+argc)*8;
		
		load_base(get_phys_addr((uint64_t)pml4_t)); /* loaded process page tables */
		memcpy((void *)sp,(void *)ustack,(3+argc)*8);
		load_base((uint64_t)pml4_base); /* load kernel page tables */
		
		
		/* save name */
		strcpy(proc->name,path);
		/* commit to the user image */
		old_pml4_t=proc->pml4_t;
		proc->pml4_t=pml4_t;
		proc->size=sz;				/* in bytes */
		proc->tf->rip=elf->e_entry;	/* main */
		proc->tf->rsp=sp;
		clear_gpr(proc->tf);		/* clear only the gprs of trapframe */
		switchuvm(proc);
		free_vma_list(&proc->vma_head);		/* free the old vmas */
		proc->vma_head=head;		/* store head of vma's in proc */
		free_uvm(old_pml4_t);
		return 0;
	}
	/* else return -ENOEXEC */
	else{
		return -ENOEXEC;
	}

}
