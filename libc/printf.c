#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void itoa(int number, char *str, int base);

int printf(const char *format, ...) {
	va_list val;
	int printed = 0, number;
	char *string = NULL;
	char numberString[10];

	va_start(val, format);

	while(*format) {

		if(*format == '%')
		{	
			++format;
			if(*format == 's')
			{
				++format;
				string = va_arg(val, char *);
				printf(string);
			}
			/*else if(*format == 'c')
			{
				++format;
				string = va_arg(val, char *);
				printf(string);
			}*/
			//TODO add printf for unsigned long
			else if(*format == 'd')
			{
				++format;
				number = va_arg(val, int);
				itoa(number, numberString, 10);
				printf(numberString);
			}
			else if(*format == 'x')
			{
				++format;
				number = va_arg(val, int);
				itoa(number, numberString, 16);
				printf(numberString);
			}

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
