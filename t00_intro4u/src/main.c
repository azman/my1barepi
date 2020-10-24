/*----------------------------------------------------------------------------*/
#include "raspi.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
#define GPIO_OFFSET 0x00200000
/*----------------------------------------------------------------------------*/
#define GPIO_BASE (PMAP_BASE|GPIO_OFFSET)
#define GPIO_FSEL (GPIO_BASE+0x00)
#define GPIO_FSET (GPIO_BASE+0x1C)
#define GPIO_FCLR (GPIO_BASE+0x28)
/*----------------------------------------------------------------------------*/
#define GPIO_ACT_LED 47
#define COUNT_MAX 0x200000
/*----------------------------------------------------------------------------*/
void main(void)
{
	/** configure gpio as output */
	put32(GPIO_FSEL+((GPIO_ACT_LED/10)*4),1<<(GPIO_ACT_LED%10)*3);
	/** main loop */
	while(1)
	{
		/** logic hi */
		put32(GPIO_FSET+((GPIO_ACT_LED/32)*4),1<<(GPIO_ACT_LED%32));
		loopd(COUNT_MAX);
		/** logic lo */
		put32(GPIO_FCLR+((GPIO_ACT_LED/32)*4),1<<(GPIO_ACT_LED%32));
		loopd(COUNT_MAX);
	}
}
/*----------------------------------------------------------------------------*/
