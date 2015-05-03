#ifndef __SBUNIX_H
#define __SBUNIX_H

#include <sys/defs.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

void printf(const char *fmt, ...);
enum { O_RDONLY = 0, O_WRONLY = 1, O_RDWR = 2, O_CREAT = 0x40, O_DIRECTORY = 0x10000 };
/* declarations for console driver */
/* defined in /sys/printf.c */
void putchars(const char *str);
void putch(char c);
void move_cursor();
void scroll_down();
void clear_screen();
void clear_line(int line);
void clear_kbdglyph();
void set_color(int c);


void timer_handler();
void keyboard_handler();
void print_time(int, int, int);
void print_char(char);
void init_timer(int);

#endif
