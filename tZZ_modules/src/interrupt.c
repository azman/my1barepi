/*----------------------------------------------------------------------------*/
#include "interrupt.h"
/*----------------------------------------------------------------------------*/
#define INTR_BASE (PMAP_BASE|INTR_OFFSET)
/*----------------------------------------------------------------------------*/
#define INTR_PENDING_BASE      (INTR_BASE+0x200)
#define INTR_PENDING_BASIC     (INTR_BASE+0x200)
#define INTR_PENDING_1         (INTR_BASE+0x204)
#define INTR_PENDING_2         (INTR_BASE+0x208)
#define INTR_FIQ_CTRL          (INTR_BASE+0x20C)
#define INTR_ENABLE_BASE       (INTR_BASE+0x210)
#define INTR_ENABLE_IRQ1       (INTR_BASE+0x210)
#define INTR_ENABLE_IRQ2       (INTR_BASE+0x214)
#define INTR_ENABLE_BASIC_IRQ  (INTR_BASE+0x218)
#define INTR_DISABLE_BASE      (INTR_BASE+0x21C)
#define INTR_DISABLE_IRQ1      (INTR_BASE+0x21C)
#define INTR_DISABLE_IRQ2      (INTR_BASE+0x220)
#define INTR_DISABLE_BASIC_IRQ (INTR_BASE+0x224)
/*----------------------------------------------------------------------------*/
void interrupt_enable(int set,unsigned int sel)
{
	unsigned addr = INTR_ENABLE_BASE+(set<<2);
	unsigned data = get32(addr);
	put32(addr,data|sel);
}
/*----------------------------------------------------------------------------*/
void interrupt_disable(int set,unsigned int sel)
{
	unsigned addr = INTR_DISABLE_BASE+(set<<2);
	unsigned data = get32(addr);
	put32(addr,data|sel);
}
/*----------------------------------------------------------------------------*/
unsigned int interrupt_pending(int set,unsigned int mask)
{
	return get32((INTR_PENDING_BASE+(set<<2))&mask);
}
/*----------------------------------------------------------------------------*/
