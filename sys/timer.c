#include <sys/sbunix.h>
#include <sys/utility.h>
#define BUFF_SIZE 20

int timer_ticks = 0;
int hh = 0, mm = 0, ss = 0;

void timer_handler(){

	char s[BUFF_SIZE] = "Timer";

	timer_ticks++;
	if((timer_ticks % 100) == 0){
		timer_ticks = 0;
		//printf("One second passed\n");
		ss++;
		if(ss % 60 == 0)
		{
			mm++;
			ss = 0;
		}
		if((mm>0) && (mm % 60 == 0))
		{
			hh++;
			mm = 0;
		}
		
		printf("%d:%d:%d\n", hh,mm,ss);
		print_time(s);

/*TODO:	Print time on the bottom right corner in the format 00:00:00 (HH:MM:SS)
		Implement RTC afer everything is done*/
	}
}


void print_time(char *s){
	 char *v;
	for(v = (char*)0xb8f00; *s; ++s, v += 2) *v = *s;
}

void init_timer(int hz){
	int divisor = 1193180 / hz;       /* Calculate our divisor */
    outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}


