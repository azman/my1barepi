/*----------------------------------------------------------------------------*/
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define MY_LED 2
#define MY_SWITCH 3
#define MY_TESTPULL 4
/*----------------------------------------------------------------------------*/
#define COUNT_MAX 0x200000
/*----------------------------------------------------------------------------*/
/* function defined in boot-intro.s */
void loopd(unsigned int);
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int data = 0xAA;
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	gpio_config(MY_SWITCH,GPIO_INPUT);
	gpio_config(MY_TESTPULL,GPIO_INPUT);
	gpio_init_data(GPIO_OUTPUT);
	gpio_clr(MY_LED);
	gpio_pull(MY_TESTPULL,GPIO_PULL_DOWN);
	gpio_put_data(data);
	while(1)
	{
		if(gpio_read(MY_SWITCH)) continue;
		gpio_set(MY_LED);
		gpio_pull(MY_TESTPULL,GPIO_PULL_UP);
		gpio_put_data((data^=0xFF)); /* toggle data */
		loopd(COUNT_MAX);
		gpio_clr(MY_LED);
		gpio_pull(MY_TESTPULL,GPIO_PULL_DOWN);
		gpio_put_data((data^=0xFF));
		loopd(COUNT_MAX);
	}
}
/*----------------------------------------------------------------------------*/
