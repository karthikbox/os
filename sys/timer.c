#include <sys/sbunix.h>
#include <sys/utility.h>
#include<sys/process.h>
#define BUFF_SIZE 20

int timer_ticks = 0;
int hh = 0, mm = 0, ss = 0;

void timer_handler(){

/*TODO:	Implement RTC afer everything is done*/
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
		
		print_time(hh,mm,ss);
		update_sleep_queue();
	}
}


//function to change the frequency of the timer
//timer ticks 100 times per second
void init_timer(int hz){
	int divisor = 1193180 / hz;       /* Calculate our divisor */
    outportb(0x43, 0x36);             /* Set our command byte 0x36 */
    outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}


