/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define TIMER_BASE (PMAP_BASE|TIMER_OFFSET)
#define TIMER_SYS_BASE (PMAP_BASE|TIMER_SYS_OFFSET)
/*----------------------------------------------------------------------------*/
#include "timer.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
#define TIMER_LOAD (TIMER_BASE+0x00)
#define TIMER_DVAL (TIMER_BASE+0x04)
#define TIMER_CTRL (TIMER_BASE+0x08)
#define TIMER_IRQA (TIMER_BASE+0x0C)
#define TIMER_IRQR (TIMER_BASE+0x10)
#define TIMER_IRQM (TIMER_BASE+0x14)
#define TIMER_RLOD (TIMER_BASE+0x18)
#define TIMER_PDIV (TIMER_BASE+0x1C)
#define TIMER_CVAL (TIMER_BASE+0x20)
/*----------------------------------------------------------------------------*/
#define TIMER_BITS_16 0x00000000
#define TIMER_BITS_23 0x00000002
#define TIMER_PRESCALE_NONE 0x00000000
#define TIMER_PRESCALE_OV16 0x00000004
#define TIMER_PRESCALE_O256 0x00000008
#define TIMER_PRESCALE_OVR1 0x0000000C
/*----------------------------------------------------------------------------*/
/** [TODO] check values for pi 2/3! */
/** prescaler = 0xF9, freq=sys_clk/(prescaler+1)*/
#define TIMER_FRC_SCALE_1MHZ 0x00F90000
/** predivider = 0xF9 (10-bits), timer_clk=apb_clk/(predivider+1) */
#define TIMER_TMR_SCALE_1MHZ 0x000000F9
/*----------------------------------------------------------------------------*/
#define TIMER_FRC_ENABLE 0x00000200
#define TIMER_TMR_ENABLE 0x00000080
#define TIMER_IRQ_ENABLE 0x00000020
/*----------------------------------------------------------------------------*/
void timer_init(void)
{
	put32(TIMER_CTRL,TIMER_BITS_23 | TIMER_PRESCALE_NONE |
		TIMER_FRC_SCALE_1MHZ | TIMER_FRC_ENABLE);
	put32(TIMER_PDIV,TIMER_TMR_SCALE_1MHZ);
}
/*----------------------------------------------------------------------------*/
unsigned int timer_read(void)
{
	return get32(TIMER_CVAL);
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
	unsigned int data = get32(TIMER_CTRL);
	if(enable) data |= TIMER_IRQ_ENABLE;
	else data &= ~TIMER_IRQ_ENABLE;
	put32(TIMER_CTRL,data);
}
/*----------------------------------------------------------------------------*/
void timer_active(int enable)
{
	unsigned int data = get32(TIMER_CTRL);
	if(enable) data |= TIMER_TMR_ENABLE;
	else data &= ~TIMER_TMR_ENABLE;
	put32(TIMER_CTRL,data);
}
/*----------------------------------------------------------------------------*/
void timer_load(unsigned int value)
{
	put32(TIMER_LOAD,value);
}
/*----------------------------------------------------------------------------*/
void timer_reload(unsigned int value)
{
	/** copy of timer load reg - does not immediately write to timer value */
	put32(TIMER_RLOD,value);
}
/*----------------------------------------------------------------------------*/
void timer_irq_clear(void)
{
	put32(TIMER_IRQA,0); /* write any value? must it be zero? */
}
/*----------------------------------------------------------------------------*/
unsigned int timer_value(void)
{
	return get32(TIMER_DVAL);
}
/*----------------------------------------------------------------------------*/
unsigned int timer_irq_raw(void)
{
	return get32(TIMER_IRQR);
}
/*----------------------------------------------------------------------------*/
unsigned int timer_irq_masked(void)
{
	return get32(TIMER_IRQM);
}
/*----------------------------------------------------------------------------*/
/* prepate for codes using system timer... more accurate? */
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
