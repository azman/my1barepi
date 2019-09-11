/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
#define GPIO_LED 2
#define GPIO_SWI 3
#define COUNT_MAX 0x200000
/*----------------------------------------------------------------------------*/
void main(void)
{
	gpio_init();
	gpio_config(GPIO_SWI,GPIO_INPUT); /* configure as input */
	gpio_pull(GPIO_SWI,GPIO_PULL_NONE); /* disable all internal pulls */
	gpio_config(GPIO_LED,GPIO_OUTPUT); /* configure as output */
	gpio_clr(GPIO_LED);
	while(1)
	{
		if(gpio_read(GPIO_SWI)) continue;
		gpio_set(GPIO_LED);
		loopd(COUNT_MAX);
		gpio_clr(GPIO_LED);
		loopd(COUNT_MAX);
	}
}
/*----------------------------------------------------------------------------*/
