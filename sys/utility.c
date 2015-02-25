#include <sys/defs.h>
#include <sys/utility.h>


void *memcpy(void *dest,const void *src,size_t len){
	/* copies len bytes of src memory into dest memory */
	size_t i=0;
	char *d=dest;
	const char *s=src;
	for(;i<len;i++){
		*d++=*s++;
	}
	return (void *)dest;
}

void *memset1(char *dest,int c,size_t len){
	/* memset() function fills the first len bytes of memory area pointed to by dest with the constant shortint c */
	size_t i=0;
	char *d=dest;
	for(;i<len;i++){
		*d++=(char)c;//write one byte of memory
	}
	return dest;
}


void *memset2(uint16_t *dest,int c,size_t len){
	/* memset() function fills the first len shortint of memory area pointed to by dest with the constant shortint c */
	size_t i=0;
	uint16_t *d=dest;
	for(;i<len;i++){
		*d++=c;
	}
	return dest;
}

void outportb(unsigned short _port, unsigned char _data) {

	__asm__ __volatile__ ("outb %1, %0"
	  :
	  : "dN" (_port),
	    "a" (_data));
}
