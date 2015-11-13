/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "interrupt.h"
/*----------------------------------------------------------------------------*/
#define TEST_PIN 47
#define TEST_DELAY 500
/*----------------------------------------------------------------------------*/
void irq_handler(void)
{
	gpio_toggle(TEST_PIN);
	timer_irq_clear();
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	interrupt_init();
	interrupt_disable(INTR_IRQSET0,INTR_BASIC_TIMER);
	gpio_init();
	gpio_config(TEST_PIN,GPIO_OUTPUT);
	timer_init();
	timer_load(TEST_DELAY*TIMER_MS);
	timer_reload(TEST_DELAY*TIMER_MS);
	timer_irq_clear();
	/** ready to go */
	enable_irq();
	interrupt_enable(INTR_IRQSET0,INTR_BASIC_TIMER);
	timer_setirq(1);
	timer_active(1);
	while(1)
	{
		/** let's do nothing */
	}
}
/*----------------------------------------------------------------------------*/
