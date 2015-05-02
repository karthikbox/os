#ifndef _UTILITY_H
#define _UTILITY_H

void *memcpy(void *dest,const void *src,size_t len);
void *memset1(char *dest,int c,size_t len);
void *memset2(uint16_t *dest,int c,size_t len);

void outportb(unsigned short _port, unsigned char _data);
unsigned char inportb (unsigned short _port);

void itoa32(int number, char *str, int base);
void itoa64(uint64_t number, char *str, int base);

/* declarations for varargs */
/* copied from include/varargs.h */
#define va_start(v,l)     __builtin_va_start(v,l)
#define va_arg(v,l)       __builtin_va_arg(v,l)
#define va_end(v)         __builtin_va_end(v)
#define va_copy(d,s)      __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;

int strlen(char *s);
int strcmp(const char *s1,const char * s2);
void strcat(char* envPath, char* path);
void strcpy(char* dest, char* src);
char** strtoken(const char *s, const char *delim,int *len);
void free_array(char **tokens,int len);
#endif
