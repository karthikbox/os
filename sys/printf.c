#include <sys/sbunix.h>
#include <sys/utility.h>

/* console driver helper definitions */
#define BLACK_ON_BLACK 0x00 //black letters on black background
#define SPACE (' '|(BLACK_ON_BLACK<<8))

static int x_con=0;
static int y_con=0;
static int time_x=0;
static int kbd_x=0;
static int color = 0x0F; //white letters on black background
static uint16_t *vga_buf=(uint16_t *)0xB8000;
static uint16_t *time_vga_buf=(uint16_t *)0xB8f82;
static uint16_t *kbd_vga_buf=(uint16_t *)0xB8f96;

static inline int serial_addr(){
	return y_con*80+x_con;
}


/* printf helper declarations */
void itoa32(int number, char *str, int base);
void itoa64(uint64_t number, char *str, int base);
void itoa64_s(long number, char *str, int base);
void put_time_chars(int tt);
void put_colon(char col);
#define BUFF_SIZE 20


void printf(const char *format, ...){
	va_list val;
	int printed = 0, number;
	char *string = NULL;
	char numberString[BUFF_SIZE];
	char c[2];
	uint64_t temp=0;
	long t=0;

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
			else if(*format == 'l'){
				t = va_arg(val, long);
				itoa64_s(t, numberString, 10);
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

//print time on the bottom right corner
//prints in the format of hh:mm:ss
void print_time(int hh, int mm, int ss)
{
	char col = ':';

	//clear the space before printing
	memset2(time_vga_buf,SPACE,8);

	put_time_chars(hh);
	put_colon(col);
	put_time_chars(mm);
	put_colon(col);
	put_time_chars(ss);

	//reset the position of x co-ordinate of time
	time_x=0;
}

void put_time_chars(int tt){
	int i = 0;
	char numberString[BUFF_SIZE];
	itoa32(tt, numberString, 10);
	uint16_t* pos=NULL;
	while(numberString[i] != '\0')
	{
		pos= time_vga_buf + time_x;
		*pos=numberString[i]|(color<<8);
		time_x++;
		i++;
	}
}

void put_colon(char col){
	uint16_t* pos= time_vga_buf + time_x;
	*pos=col|(color<<8);
	time_x++;
}

//print the last pressed glyph next to clock
void print_char(char c){
	
	uint16_t* pos= kbd_vga_buf + kbd_x;
	*pos=c|(color<<8);
	kbd_x++;

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

void itoa64_s(long number, char *str, int base)
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
	while(y_con>=24){
		/* pull the all lines back by one line. Now cursor is in the last line */
		memcpy(vga_buf,vga_buf+80,80*23*2);
		/* y_con now is at last line */
		y_con--;
		/* make last line empty -> fill it with space -> looks like empty */
		/* memset sets every 2bytes to space */
		memset2(vga_buf+serial_addr(),SPACE,80);
	}
}

void clear_screen(){
	/* set all the VGA buffer memory to space character with fg and bg black and reset cursor to top leftof screen */
	memset2(vga_buf,SPACE,80*24);
	x_con=0;
	y_con=0;
}

void clear_line(int k){
	// line index starts from 0 to 24
	//k=0 first line
	//k=24 last line
	uint16_t *target=vga_buf+k*80;
	memset2(target,SPACE,80);
}

void clear_kbdglyph(){

	//clear the space before printing
	memset2(kbd_vga_buf,SPACE,2);
	kbd_x = 0;

}


void set_color(int c){
	color=c;
}
