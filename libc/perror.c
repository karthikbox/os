#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void itoa(int number, char *str, int base);
void printarg(char *str,int fd);



int perror(const char *format, ...) {
	va_list val;
	int printed = 0, number;
	char *string = NULL;
	char numberString[10];
	char c[2];
	va_start(val, format);

	while(*format) {

		if(*format == '%')
		{	
			++format;
			if(*format == 's')
			{
				string = va_arg(val, char *);
				printarg(string,2);
			}
			else if(*format == 'c')
			{
				char d = va_arg(val, int);
				c[0]=d;
				c[1]='\0';
				printarg(c,2);
			}
			//TODO add printf for unsigned long
			else if(*format == 'd')
			{
				number = va_arg(val, int);
				itoa(number, numberString, 10);
				printarg(numberString,2);
			}
			else if(*format == 'x')
			{
				number = va_arg(val, int);
				itoa(number, numberString, 16);
				printarg(numberString,2);
			}

			++format;
			if(*format == '\0')
				break;

		}
		write(2, format, 1);
		++printed;
		++format;
	}

	va_end(val);

	return printed;
}


