/*----------------------------------------------------------------------------*/
#include "gpio.h"
/*----------------------------------------------------------------------------*/
/** TIMER MODULE BEGIN - THESE FUNCTIONS WILL BE MADE INTO A LIBRARY! */
/*----------------------------------------------------------------------------*/
#define TIMER_BASE 0x2000B000
/** offset 0x400 */
#define TIMER_CTRL 0x102
#define TIMER_CVAL 0x108
/*----------------------------------------------------------------------------*/
/** free-running counter enable bit position */
#define TIMER_FRC_ENABLE 0x00000200
/*----------------------------------------------------------------------------*/
/* timer_wait is usleep equivalent */
#define TIMER_MS 1000
#define TIMER_S 1000000
/*----------------------------------------------------------------------------*/
volatile unsigned int *timer;
/*----------------------------------------------------------------------------*/
void timer_init(void)
{
	timer = (unsigned int*) TIMER_BASE;
	/** prescaler = 0xF9, freq=sys_clk/(prescaler+1)*/
	timer[TIMER_CTRL] = 0x00F90000; /** for 1MHz? sys_clk is 250MHz? */
	timer[TIMER_CTRL] |= TIMER_FRC_ENABLE;
}
/*----------------------------------------------------------------------------*/
unsigned int timer_read(void)
{
	return timer[TIMER_CVAL];
}
/*----------------------------------------------------------------------------*/
void timer_wait(unsigned int wait)
{
	unsigned int init = timer_read();
	while((timer_read()-init)<wait);
}
/*----------------------------------------------------------------------------*/
/** TIMER MODULE END */
/*----------------------------------------------------------------------------*/
#define TEST_PIN 2
#define TEST_DELAY 500
/*----------------------------------------------------------------------------*/
void main(void)
{
	gpio_init();
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
