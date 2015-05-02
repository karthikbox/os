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

unsigned char inportb (unsigned short _port)
{
	unsigned char rv;
	__asm__ __volatile__ ("inb %1, %0" 
		: "=a" (rv) : 
		"dN" (_port));
	
	return rv;
}

int strcmp(const char *s1,const char * s2) {
    int i=0;
    for(i=0; s1[i]==s2[i]; i++) {
        if(s1[i]=='\0')
            return 0;
    }
    return s1[i]-s2[i];
}



int strlen(char *s)
{
    int i = 0;
    while(s[i] != '\0')
        i++;
    return i;

}

void strcat(char* s1, char* s2)
{
    int i=0, j=0;
    while (s1[i] != '\0')
    {
        i++;
    }
    while (s2[j] != '\0')
    {
        s1[i] = s2[j];
        i++;
        j++;
    }
    s1[i] = '\0';
}

void strcpy(char* dest, char* src)
{
    int i=0;
    while(src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}


