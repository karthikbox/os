#ifndef __STRING_H
#define _STRING_H

#include<sys/defs.h>

int strcmp(const char *s1,const char *s2);
char** strtoken(const char *s, const char *delim,int *len);
int strlen(char *s);
void strcat(char* s1, char* s2);
void strcpy(char* dest, char* src);
#endif
