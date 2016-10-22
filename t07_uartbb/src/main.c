/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uartbb.h"
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int test;
	gpio_init();
	timer_init();
	uartbb_init(UARTBB_RX_DEFAULT,UARTBB_TX_DEFAULT);
	/** send out the word! */
	uartbb_print("\nYAY! THIS WORKS!\n");
	/** do the thing... */
	while(1)
	{
		/** simple echo! */
		test = uartbb_read();
		uartbb_send(test);
	}
}
/*----------------------------------------------------------------------------*/
