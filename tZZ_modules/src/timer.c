/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define TIMER_BASE (PMAP_BASE|TIMER_OFFSET)
#define TIMER_SYS_BASE (PMAP_BASE|TIMER_SYS_OFFSET)
/*----------------------------------------------------------------------------*/
#include "timer.h"
/*----------------------------------------------------------------------------*/
#define TIMER_LOAD 0x00
#define TIMER_DVAL 0x01
#define TIMER_CTRL 0x02
#define TIMER_IRQA 0x03
#define TIMER_IRQR 0x04
#define TIMER_IRQM 0x05
#define TIMER_RLOD 0x06
#define TIMER_PDIV 0x07
#define TIMER_CVAL 0x08
/*----------------------------------------------------------------------------*/
#define TIMER_BITS_16 0x00000000
#define TIMER_BITS_23 0x00000002
#define TIMER_PRESCALE_NONE 0x00000000
#define TIMER_PRESCALE_OV16 0x00000004
#define TIMER_PRESCALE_O256 0x00000008
#define TIMER_PRESCALE_OVR1 0x0000000C
/*----------------------------------------------------------------------------*/
/** prescaler = 0xF9, freq=sys_clk/(prescaler+1)*/
#define TIMER_FRC_SCALE_1MHZ 0x00F90000
/** predivider = 0xF9 (10-bits), timer_clk=apb_clk/(predivider+1) */
#define TIMER_TMR_SCALE_1MHZ 0x000000F9
/*----------------------------------------------------------------------------*/
#define TIMER_FRC_ENABLE 0x00000200
#define TIMER_TMR_ENABLE 0x00000080
#define TIMER_IRQ_ENABLE 0x00000020
/*----------------------------------------------------------------------------*/
#define TIMER_SYS_CTRL 0
#define TIMER_SYS_STAT 0
#define TIMER_SYS_CNTL 1
#define TIMER_SYS_CNTH 2
#define TIMER_SYS_CMP0 3
#define TIMER_SYS_CMP1 4
#define TIMER_SYS_CMP2 5
#define TIMER_SYS_CMP3 6
/*----------------------------------------------------------------------------*/
#define TIMER_SYS_MATCH0 0x00000001
#define TIMER_SYS_MATCH1 0x00000002
#define TIMER_SYS_MATCH2 0x00000004
#define TIMER_SYS_MATCH3 0x00000008
/*----------------------------------------------------------------------------*/
volatile unsigned int *timer;
/*----------------------------------------------------------------------------*/
void timer_init(void)
{
	timer = (unsigned int*) TIMER_BASE;
	timer[TIMER_CTRL] = TIMER_BITS_23 | TIMER_PRESCALE_NONE |
		TIMER_FRC_SCALE_1MHZ | TIMER_FRC_ENABLE;
	timer[TIMER_PDIV] = TIMER_TMR_SCALE_1MHZ;
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
void timer_setirq(int enable)
{
	if(enable) timer[TIMER_CTRL] |= TIMER_IRQ_ENABLE;
	else timer[TIMER_CTRL] &= ~TIMER_IRQ_ENABLE;
}
/*----------------------------------------------------------------------------*/
void timer_active(int enable)
{
	if(enable) timer[TIMER_CTRL] |= TIMER_TMR_ENABLE;
	else timer[TIMER_CTRL] &= ~TIMER_TMR_ENABLE;
}
/*----------------------------------------------------------------------------*/
void timer_load(int value)
{
	timer[TIMER_LOAD] = value;
}
/*----------------------------------------------------------------------------*/
void timer_reload(int value)
{
	timer[TIMER_RLOD] = value;
}
/*----------------------------------------------------------------------------*/
void timer_irq_clear(void)
{
	timer[TIMER_IRQA] = 0; /* write any value? must it be zero? */
}
/*----------------------------------------------------------------------------*/
unsigned int timer_irq_raw(void)
{
	return timer[TIMER_IRQR];
}
/*----------------------------------------------------------------------------*/
unsigned int timer_irq_masked(void)
{
	return timer[TIMER_IRQM];
}
/*----------------------------------------------------------------------------*/
