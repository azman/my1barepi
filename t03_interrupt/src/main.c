/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
/** INTERRUPT MODULE BEGIN - THESE FUNCTIONS WILL BE MADE INTO A LIBRARY! */
/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define INTR_BASE (PMAP_BASE|INTR_OFFSET)
/*----------------------------------------------------------------------------*/
#define INTR_PENDING_BASE 0x00
#define INTR_FIQ_CTRL 0x03
#define INTR_ENABLE_BASE 0x04
#define INTR_DISABLE_BASE 0x07
/*----------------------------------------------------------------------------*/
#define INTR_IRQSET1 0
#define INTR_IRQSET2 1
#define INTR_IRQSET0 2
#define INTR_IRQCHK0 0
#define INTR_IRQCHK1 1
#define INTR_IRQCHK2 2
/*----------------------------------------------------------------------------*/
#define INTR_BASIC_TIMER 0x00000001
/* GPIO interrupts @IRQ49-52 => BIT17-20 */
#define INTR_PEND2_GPIOS 0x001E0000
/*----------------------------------------------------------------------------*/
volatile unsigned int *interrupt;
/*----------------------------------------------------------------------------*/
void interrupt_init(void)
{
	interrupt = (unsigned int*) INTR_BASE;
}
/*----------------------------------------------------------------------------*/
void interrupt_enable(int set,unsigned int sel)
{
	interrupt[INTR_ENABLE_BASE+set] |= sel;
}
/*----------------------------------------------------------------------------*/
void interrupt_disable(int set,unsigned int sel)
{
	interrupt[INTR_DISABLE_BASE+set] |= sel;
}
/*----------------------------------------------------------------------------*/
unsigned int interrupt_pending(int set,unsigned int mask)
{
	return (interrupt[INTR_PENDING_BASE+set]&mask);
}
/*----------------------------------------------------------------------------*/
void enable_irq(void); /** defined in assembly source file */
/*----------------------------------------------------------------------------*/
/** INTERRUPT MODULE END */
/*----------------------------------------------------------------------------*/
#define INTR_PIN 2
#define TEST_PIN 47
#define TEST_DELAY 500
/*----------------------------------------------------------------------------*/
int pause = 0;
/*----------------------------------------------------------------------------*/
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
	gpio_init();
	gpio_config(TEST_PIN,GPIO_OUTPUT);
	gpio_config(INTR_PIN,GPIO_INPUT);
	gpio_pull(INTR_PIN,GPIO_PULL_UP);
	gpio_rstevent(INTR_PIN);
	gpio_setevent(INTR_PIN,GPIO_EVENT_AEDGR);
	timer_init();
	timer_load(TEST_DELAY*TIMER_MS);
	timer_reload(TEST_DELAY*TIMER_MS);
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
