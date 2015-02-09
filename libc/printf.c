#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int printf(const char *format, ...) {
	va_list val;
	int printed = 0;

	va_start(val, format);

	while(*format) {
	    if(write(1, format, 1)==-1)
		return -1;
	    ++printed;
	    ++format;
	}

	return printed;
}
