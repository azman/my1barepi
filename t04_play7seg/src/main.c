/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "7seg.h"
/*----------------------------------------------------------------------------*/
#define MY_SWI 15
/*----------------------------------------------------------------------------*/
int segment_numbers[10][SEGMENT_COUNT] =
{
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF }, /* 0 */
	{ LED_OFF,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_OFF,LED_OFF }, /* 1 */
	{ LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON,LED_OFF,LED_ON }, /* 2 */
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_ON }, /* 3 */
	{ LED_OFF,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_ON,LED_ON }, /* 4 */
	{ LED_ON,LED_OFF,LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON }, /* 5 */
	{ LED_ON,LED_OFF,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON }, /* 6 */
	{ LED_ON,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_OFF,LED_OFF }, /* 7 */
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON }, /* 8 */
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON }  /* 9 */
};
/*----------------------------------------------------------------------------*/
void main(void)
{
	seven_seg_t seg7 = {0x0,{2,3,4,17,27,22,10},9,SEGMENT_COMMON_ANODE};
	int count;
	gpio_init();
	gpio_config(MY_SWI,GPIO_INPUT);
	timer_init();
	/* init 7segment interface */
	seven_init(&seg7,&segment_numbers[0][0]);
	while(1)
	{
		seven_show(&seg7,0);
		if(gpio_read(MY_SWI)) continue;
		for(count=0;count<10;count++)
		{
			seven_show(&seg7,count);
			timer_wait(TIMER_S);
		}
	}
}
/*----------------------------------------------------------------------------*/
