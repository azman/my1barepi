/*----------------------------------------------------------------------------*/
#include "timer.h"
/*----------------------------------------------------------------------------*/
#define TIMER_BASE 0x2000B000
/** offset 0x400 */
#define TIMER_CTRL 0x102
#define TIMER_CVAL 0x108
/*----------------------------------------------------------------------------*/
/** free-running counter enable bit position */
#define TIMER_FRC_ENABLE 0x00000200
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
