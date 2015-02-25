#ifndef _UTILITY_H
#define _UTILITY_H

void *memcpy(void *dest,const void *src,size_t len);
void *memset1(char *dest,int c,size_t len);
void *memset2(uint16_t *dest,int c,size_t len);

void outportb(unsigned short _port, unsigned char _data);

/* declarations for varargs */
/* copied from include/varargs.h */
#define va_start(v,l)     __builtin_va_start(v,l)
#define va_arg(v,l)       __builtin_va_arg(v,l)
#define va_end(v)         __builtin_va_end(v)
#define va_copy(d,s)      __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

#endif
