/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
void delay_this(unsigned int wait)
{
	unsigned int init = timer_read();
	while((timer_read()-init)<wait);
}
/*----------------------------------------------------------------------------*/
#define TEST_PIN 2
#define TEST_DELAY 500
/*----------------------------------------------------------------------------*/
void main(void)
{
	int count = 0;
	gpio_init();
	timer_init();
	gpio_config(TEST_PIN,GPIO_OUTPUT);
	while(1)
	{
		gpio_set(TEST_PIN);
		delay_this(TEST_DELAY);
		gpio_clr(TEST_PIN);
		delay_this(TEST_DELAY);
	}
}
/*----------------------------------------------------------------------------*/
