/*----------------------------------------------------------------------------*/
#include "interrupt.h"
/*----------------------------------------------------------------------------*/
#ifdef RASPI2
#define INTR_BASE 0x3F00B200
#else
#define INTR_BASE 0x2000B200
#endif
/*----------------------------------------------------------------------------*/
#define INTR_PENDING_BASE 0x00
#define INTR_FIQ_CTRL 0x03
#define INTR_ENABLE_BASE 0x04
#define INTR_DISABLE_BASE 0x07
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
