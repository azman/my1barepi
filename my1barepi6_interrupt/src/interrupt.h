/*----------------------------------------------------------------------------*/
#ifndef __MY1INTERRUPTH__
#define __MY1INTERRUPTH__
/*----------------------------------------------------------------------------*/
#define INTR_IRQSET1 0
#define INTR_IRQSET2 1
#define INTR_IRQSET0 2
#define INTR_IRQCHK0 0
#define INTR_IRQCHK1 1
#define INTR_IRQCHK2 2
/*----------------------------------------------------------------------------*/
#define INTR_BASIC_TIMER 0x00000001
/*----------------------------------------------------------------------------*/
void interrupt_init(void);
void interrupt_enable(int set,unsigned int sel);
void interrupt_disable(int set,unsigned int sel);
unsigned int interrupt_pending(int set,unsigned int mask);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
