/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
#define TEST_PIN 26
#define TEST_DELAY 500
/*----------------------------------------------------------------------------*/
void main(void)
{
	timer_init();
	gpio_config(TEST_PIN,GPIO_OUTPUT);
	while(1)
	{
		gpio_set(TEST_PIN);
		timer_wait(TEST_DELAY);
		gpio_clr(TEST_PIN);
		timer_wait(TEST_DELAY);
	}
}
/*----------------------------------------------------------------------------*/
