#include<sys/sbunix.h>

static int x_con=0;
static int y_con=0;
static int color = 0x0F; //white letters on black background
static const uint16_t *vga_buf=(unit16_t *)0xB8000;

int inline serial_addr(){
	return y_con*80+x_con;
}


void putchars(char *str);
void putch(char c);
void move_cursor();
void scroll_down();

void printf(const char *fmt, ...){
	
	putchars(str);
}

void putchars(char *str){
	while(*str!='\0'){
		putch(*str);
	}
	move_cursor();
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

	while(y>=25){
		/* only if y has exceeded screen dimensions */
		memcpy(vga_buf,vga_buf+(uint16_t*)80,80*24*2);
		y--;
	}
}

