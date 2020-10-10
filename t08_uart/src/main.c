/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int test;
	gpio_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	/** send out the word! */
	uart_print("\nYAY! THIS WORKS!\n");
	/** do the thing... */
	while(1)
	{
		if(uart_incoming())
		{
			/** simple echo! */
			test = uart_read();
			uart_send(test);
		}
	}
}
/*----------------------------------------------------------------------------*/
