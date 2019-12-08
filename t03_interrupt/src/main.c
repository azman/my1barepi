/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "interrupt.h"
/*----------------------------------------------------------------------------*/
#define TEST_PIN 2
#define TEST_DELAY 500
#define INTR_PIN 3
/*----------------------------------------------------------------------------*/
/* global parameter */
int pause = 0;
/*----------------------------------------------------------------------------*/
/* interrupt handler */
void irq_handler(void)
{
	if(gpio_chkevent(INTR_PIN))
	{
		pause = !pause;
		gpio_rstevent(INTR_PIN);
	}
	if(timer_irq_masked())
	{
		if(!pause) gpio_toggle(TEST_PIN);
		timer_irq_clear();
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	interrupt_init();
	interrupt_disable(INTR_IRQSET0,INTR_BASIC_TIMER);
	gpio_config(TEST_PIN,GPIO_OUTPUT);
	gpio_config(INTR_PIN,GPIO_INPUT);
	gpio_pull(INTR_PIN,GPIO_PULL_UP);
	gpio_rstevent(INTR_PIN);
	gpio_setevent(INTR_PIN,GPIO_EVENT_AEDGF);
	timer_init();
	timer_load(TEST_DELAY*TIMER_MS);
	timer_irq_clear();
	/** ready to go */
	enable_irq();
	interrupt_enable(INTR_IRQSET0,INTR_BASIC_TIMER);
	interrupt_enable(INTR_IRQSET2,INTR_PEND2_GPIOS);
	timer_setirq(1);
	timer_active(1);
	while(1)
	{
		/** let's do nothing */
	}
}
/*----------------------------------------------------------------------------*/
