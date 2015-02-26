#ifndef __SBUNIX_H
#define __SBUNIX_H

#include <sys/defs.h>

void printf(const char *fmt, ...);

/* declarations for console driver */
/* defined in /sys/printf.c */
void putchars(const char *str);
void putch(char c);
void move_cursor();
void scroll_down();
void clear_screen();
void set_color(int c);


void timer_handler();
void keyboard_handler();
void print_time(int, int, int);
void print_char(char);
void init_timer(int);

#endif
