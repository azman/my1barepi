/*----------------------------------------------------------------------------*/
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define MY_LED 2
#define MY_SWITCH 3
/*----------------------------------------------------------------------------*/
#define LED_ON gpio_clr
#define LED_OFF gpio_set
#define SWITCH_IS_OFF gpio_read
/*----------------------------------------------------------------------------*/
void main(void)
{
	volatile unsigned int loop; /** does not work if NOT volatile! */
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	gpio_config(MY_SWITCH,GPIO_INPUT);
	LED_OFF(MY_LED);
	while(1)
	{
		if(SWITCH_IS_OFF(MY_SWITCH)) continue;
		LED_ON(MY_LED);
		for(loop=0;loop<0x100000;loop++);
		LED_OFF(MY_LED);
		for(loop=0;loop<0x100000;loop++);
	}
}
/*----------------------------------------------------------------------------*/
