/*----------------------------------------------------------------------------*/
#ifndef __MY1INTERRUPTH__
#define __MY1INTERRUPTH__
/*----------------------------------------------------------------------------*/
/** make sure to use boot-interrupt.s */
#include "boot-interrupt.h"
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
void interrupt_enable(int set,unsigned int sel);
void interrupt_disable(int set,unsigned int sel);
unsigned int interrupt_pending(int set,unsigned int mask);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
