/*----------------------------------------------------------------------------*/
#ifndef __MY1INTERRUPTH__
#define __MY1INTERRUPTH__
/*----------------------------------------------------------------------------*/
/** make sure to use boot-interrupt.s */
#include "boot-interrupt.h"
/*----------------------------------------------------------------------------*/
/* params for interrupt_{en,dis}able (irq1,ir2,basic irq) */
#define INTR_IRQSET1 0
#define INTR_IRQSET2 1
#define INTR_IRQSET0 2
/* params for interrupt_pending (basic pend, pend1, pend2) */
#define INTR_IRQCHK0 0
#define INTR_IRQCHK1 1
#define INTR_IRQCHK2 2
/*----------------------------------------------------------------------------*/
#define INTR_BASIC_TIMER 0x00000001
#define INTR_BASIC_PEND1 0x00000100
#define INTR_BASIC_PEND2 0x00000200
/* GPIO interrupts @IRQ49-52 => BIT17-20 */
/* IRQ49 is for GPIO0-GPIO31 */
#define INTR_PEND2_IRQ49 0x00020000
#define INTR_PEND2_IRQ50 0x00040000
#define INTR_PEND2_IRQ51 0x00080000
#define INTR_PEND2_IRQ52 0x00100000
#define INTR_PEND2_GPIOS 0x001E0000
/*----------------------------------------------------------------------------*/
void interrupt_enable(int set,unsigned int sel);
void interrupt_disable(int set,unsigned int sel);
unsigned int interrupt_pending(int set,unsigned int mask);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
