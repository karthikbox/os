#ifndef __STRING_H
#define _STRING_H

#include<sys/defs.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);

#endif
