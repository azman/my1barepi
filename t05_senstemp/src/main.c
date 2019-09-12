/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "7seg.h"
#include "am2302.h"
/*----------------------------------------------------------------------------*/
#define SENS_IN 4
/*----------------------------------------------------------------------------*/
#define NUMBERS_COUNT 10
/*----------------------------------------------------------------------------*/
int numbers[NUMBERS_COUNT*SEGMENT_COUNT] =
{
	LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF, /* 0 */
	LED_OFF,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_OFF,LED_OFF, /* 1 */
	LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON,LED_OFF,LED_ON, /* 2 */
	LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_ON, /* 3 */
	LED_OFF,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_ON,LED_ON, /* 4 */
	LED_ON,LED_OFF,LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON, /* 5 */
	LED_ON,LED_OFF,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON, /* 6 */
	LED_ON,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_OFF,LED_OFF, /* 7 */
	LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON, /* 8 */
	LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON  /* 9 */
};
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int temp, humi, init;
	seven_seg_t seg1 = {numbers,{5,6,7,8,9,10,11},12,COMMON_ANODE};
	seven_seg_t seg2 = {numbers,{20,21,22,23,24,25,26},27,COMMON_ANODE};
	/* init gpio */
	gpio_init();
	gpio_config(SENS_IN,GPIO_OUTPUT);
	gpio_pull(SENS_IN,GPIO_PULL_NONE);
	gpio_set(SENS_IN);
	/* init 7segment interface */
	seven_init(&seg1);
	seven_init(&seg2);
	seven_full(&seg1,0,LED_OFF);
	seven_full(&seg2,0,LED_ON);
	/* init timer */
	timer_init();
	/* powerup - wait 1s */
	timer_wait(1000000);
	/** do the thing... */
	while(1)
	{
		/* mark sensor read time */
		init = timer_read();
		/* read sensor value(s) */
		am2302_read(SENS_IN,&temp,&humi);
		/* just display temperature for now */
		humi = humi / 10;
		temp = temp / 10;
		seven_show(&seg1,temp/10);
		seven_show(&seg2,temp%10);
		/* toggle dp - simple indicator */
		gpio_toggle(seg1.gpio_dp);
		gpio_toggle(seg2.gpio_dp);
		/** 2 seconds between read */
		while(timer_read()-init<2000000);
	}
}
/*----------------------------------------------------------------------------*/
