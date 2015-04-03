#include<sys/tarfs.h>
#include<sys/sbunix.h>
#include<sys/utility.h>
#include<sys/defs.h>
#include<string.h>


uint64_t * tarfs_get_file(char name[]){
	struct posix_header_ustar *p= (struct posix_header_ustar *)&_binary_tarfs_start;
	struct posix_header_ustar *p_e= (struct posix_header_ustar *)&_binary_tarfs_end;
	
	while(p<p_e && !(strlen(p->name)==0)){
		/* printf("cur name->%s, lookup name->%s, cur location->%p\n",p->name,name,p); */
		if(strcmp(p->name,name)==0){
			/* this is the required file name */
			return (uint64_t *)(p+1);			/* return pointer to start of ELF binary */
		}
		else{
			/* goto next file header by adding size of header and header->size */
			/* header->size should be rounded up next 512 multiple */
			//printf("%s->%u",oct_to_dec("00000 0000 0000"))
			p=(struct posix_header_ustar *)((char *)p+sizeof(struct posix_header_ustar ) + round_up(oct_to_dec(p->size),512));
		}
	}
	/* no file with that name */
	return NULL;

	
}


uint64_t oct_to_dec(char str[]){
	/* char[12] str */
	//return (uint64_t)58960;
	int i,prod=1;
	uint64_t sum=0;
	for(i=10;i>=0;i--){
		sum+=prod*(str[i]-'0');
		prod*=8;
	}
	return sum;
}


uint64_t round_up(uint64_t sz,uint64_t mul){
	if(sz%mul==0){
		return sz;
	}
	else{
		return sz + mul - (sz%mul);
	}
}


