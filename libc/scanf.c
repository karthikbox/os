#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int scanf(const char *format, ...) {

	char buf[1];
	char *string;
	va_list val;
	va_start(val, format);
	int i = 0;

	while(*format) {
		i=0;
		if(*format == '%')
		{
			//increment format to check the next character
			//it may be s, d, c or x
			++format; 
			if(*format == 's')
			{

				//store the next argument in string variable
				string = va_arg(val, char *);
				while(read(0, buf, sizeof(buf))>0) {
			   		//read() here read from stdin character by character
			   		//the buf[0] contains the character got by read()
		   			if(buf[0] == '\n')
			   		{
			   			//break the loop if input hits a new line character
			   			//also, terminate the string
			   			string[i] = '\0';
			   			break;
			   		}

			   		//store the input character by character to the input
			   		string[i] = buf[0];
			   		i++;
		   		}
			}

			//check if the first argument of scanf is terminated or not
			++format;
			if(*format == '\0')
				break;
		}
	}
	va_end(val);
	return 0;
}
