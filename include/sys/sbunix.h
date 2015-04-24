#ifndef __SBUNIX_H
#define __SBUNIX_H

#include <sys/defs.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

void printf(const char *fmt, ...);

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
