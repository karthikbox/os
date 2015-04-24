#ifndef _STDIO_H
#define _STDIO_H

#define STDIN 0
#define STDOUT 1
#define STDERR 2

int printf(const char *format, ...);
int scanf(const char *format, ...);
int perror(const char *format,...);
#endif
