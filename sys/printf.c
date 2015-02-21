#include<sys/sbunix.h>
#include<sys/utility.h>

/* console driver helper definitions */
#define BLACK_ON_BLACK 0x00 //black letters on black background
#define SPACE (' '|(BLACK_ON_BLACK<<8))

static int x_con=0;
static int y_con=0;
static int color = 0x0F; //white letters on black background
static uint16_t *vga_buf=(uint16_t *)0xB8000;

static inline int serial_addr(){
	return y_con*80+x_con;
}


/* printf helper declarations */
void itoa32(int number, char *str, int base);
void itoa64(uint64_t number, char *str, int base);
#define BUFF_SIZE 20


void printf(const char *format, ...){
	va_list val;
	int printed = 0, number;
	char *string = NULL;
	char numberString[BUFF_SIZE];
	char c[2];
	uint64_t temp=0;

	va_start(val, format);

	while(*format) {
		if(*format == '%')
		{	
			++format;
			if(*format == 's')
			{
				string = va_arg(val, char *);
				putchars(string);
			}
			else if(*format == 'c')
			{
				char d = va_arg(val, int);
				c[0]=d;
				c[1]='\0';
				putchars(c);
			}
			//TODO add printf for unsigned long
			else if(*format == 'd')
			{
				number = va_arg(val, int);
				itoa32(number, numberString, 10);
				putchars(numberString);
			}
			else if(*format == 'x')
			{
				number = va_arg(val, int);
				itoa32(number, numberString, 16);
				putchars(numberString);
			}
			else if(*format =='p'){
				temp=va_arg(val,uint64_t);
				itoa64(temp,numberString,16);
				putchars(numberString);
			}

			++format;
			if(*format == '\0')
				break;

		}
		//write(1, format, 1);
		putch(*format);
		++printed;
		++format;
	}

	va_end(val);
}

void itoa32(int number, char *str, int base)
{
	int digit, i=0, j=0;
	char temp[BUFF_SIZE];

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


void itoa64(uint64_t number, char *str, int base)
{
	int digit, i=0, j=0;
	char temp[BUFF_SIZE];

	if(number == 0)
	{
			temp[i] = '0';
			i++;
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


void putchars(const char *str){
	while(*str !='\0'){
		putch(*str++);
	}
	//move_cursor();
}

void putch(char c){

	if(c=='\n'){
		x_con=0;
		y_con++;
	}
	else if(c=='\r'){
		x_con=0;
	}
	else if(c >= ' '){
		uint16_t* pos=vga_buf+serial_addr();
		*pos=c|(color<<8);
		x_con++;
	}
	if(x_con>=80){
		x_con=0;
		y_con++;
	}
	scroll_down();
}

void scroll_down(){
	/* only if y has exceeded screen dimensions */
	while(y_con>=25){
		/* pull the all lines back by one line. Now cursor is in the last line */
		memcpy(vga_buf,vga_buf+80,80*24*2);
		/* y_con now is at last line */
		y_con--;
		/* make last line empty -> fill it with space -> looks like empty */
		/* memset sets every 2bytes to space */
		memset2(vga_buf+serial_addr(),SPACE,80);
	}
}

void clear_screen(){
	/* set all the VGA buffer memory to space character with fg and bg black and reset cursor to top leftof screen */
	memset2(vga_buf,SPACE,80*25);
	x_con=0;
	y_con=0;
}


void set_color(int c){
	color=c;
}
