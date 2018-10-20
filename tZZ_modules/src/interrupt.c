/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define INTR_BASE (PMAP_BASE|INTR_OFFSET)
/*----------------------------------------------------------------------------*/
#include "interrupt.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
#define INTR_PENDING_BASE (INTR_BASE+0x00)
#define INTR_FIQ_CTRL     (INTR_BASE+0x0C)
#define INTR_ENABLE_BASE  (INTR_BASE+0x10)
#define INTR_DISABLE_BASE (INTR_BASE+0x1C)
/*----------------------------------------------------------------------------*/
void interrupt_init(void)
{
	/* nothing to do! */
}
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
