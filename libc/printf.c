#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void itoa(int number, char *str, int base);
void printarg(char *str,int fd);

int printf(const char *format, ...) {
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
				printarg(string,1);
			}
			else if(*format == 'c')
			{
				char d = va_arg(val, int);
				c[0]=d;
				c[1]='\0';
				printarg(c,1);
			}
			//TODO add printf for unsigned long
			else if(*format == 'd')
			{
				number = va_arg(val, int);
				itoa(number, numberString, 10);
				printarg(numberString,1);
			}
			else if(*format == 'x')
			{
				number = va_arg(val, int);
				itoa(number, numberString, 16);
				printarg(numberString,1);
			}

			++format;
			if(*format == '\0')
				break;

		}
		write(1, format, 1);
		++printed;
		++format;
	}

	va_end(val);

	return printed;
}

void itoa(int number, char *str, int base)
{
	int digit, i=0, j=0;
	char temp[10];

	if(number == 0)
	{
			temp[i] = '0';
			i++;
	}

	else if(number < 0)
	{
		str[j] = '-';
		j++;
		number = -number;
	}

	while(number > 0)
	{
		digit = number%base;
		number = number/base;
		if(digit <= 9)
			temp[i] = digit + '0';
		else
			temp[i] = (digit-10) + 'a';
		i++;
	}
	temp[i]='\0';
	
	while(i >= 0)
	{
		i--;
		str[j] = temp[i];
		j++;
	}

	str[j] = '\0';
}


void printarg(char *str,int fd) {
	
	while(*str)
	{
		write(fd, str, 1);
		str++;
	}
}
