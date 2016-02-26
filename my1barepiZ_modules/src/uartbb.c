/*----------------------------------------------------------------------------*/
#include "uartbb.h"
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
/**
 * - implementing bit banging UART
 *   = fixed baud-rate at 9600 (8N1)
 *   = needs the gpio and timer module (initialized in main!)
**/
/*----------------------------------------------------------------------------*/
#define B9600_TX_DELAY 104
#define B9600_RX_DELAY 104
/*----------------------------------------------------------------------------*/
int uartbb_rx,uartbb_tx;
/*----------------------------------------------------------------------------*/
void uartbb_init(int rx_gpio,int tx_gpio)
{
	/* set the requested */
	uartbb_rx = rx_gpio;
	uartbb_tx = tx_gpio;
	/* setup pins! */
	gpio_config(uartbb_rx,GPIO_INPUT);
	gpio_config(uartbb_tx,GPIO_OUTPUT);
	/* disable pulls on tx/rx pins? */
	gpio_pull(uartbb_rx,GPIO_PULL_NONE);
	gpio_pull(uartbb_tx,GPIO_PULL_NONE);
}
/*----------------------------------------------------------------------------*/
void uartbb_send(unsigned int data)
{
	int loop;
	/* start bit */
	gpio_clr(uartbb_tx);
	timer_wait(B9600_TX_DELAY);
	for (loop=0;loop<8;loop++)
	{
		if (data&0x01) gpio_set(uartbb_tx);
		else gpio_clr(uartbb_tx);
		timer_wait(B9600_TX_DELAY);
		data >>= 1;
	}
	/* stop bit */
	gpio_set(uartbb_tx);
	timer_wait(B9600_TX_DELAY);
}
/*----------------------------------------------------------------------------*/
unsigned int uartbb_read(void)
{
	int loop;
	unsigned int data = 0x00;
	/* wait start bit */
	while(gpio_read(uartbb_rx));
	timer_wait(B9600_RX_DELAY);
	/* we read at half period */
	timer_wait(B9600_RX_DELAY/2);
	for (loop=0;loop<8;loop++)
	{
		data >>= 1;
		if(gpio_read(uartbb_rx)) data |= 0x80;
		timer_wait(B9600_RX_DELAY);
	}
	/* ignore stop bit? */
	if(!gpio_read(uartbb_rx))
		data = 0x00;
	timer_wait(B9600_RX_DELAY/2);
	return data;
}
/*----------------------------------------------------------------------------*/
void uartbb_print_hex(unsigned int value)
{
	int loop;
	unsigned int test,high;
	uartbb_send('0');
	uartbb_send('x');
	for(loop=3;loop>=0;loop--)
	{
		test = (value >> (loop*8)) & 0xff;
		high = (test & 0xf0) >> 4;
		if(high>9) high = (high-10)+0x41;
		else high += 0x30;
		uartbb_send(high);
		high = (test & 0x0f);
		if(high>9) high = (high-10)+0x41;
		else high += 0x30;
		uartbb_send(high);
	}
}
/*----------------------------------------------------------------------------*/
void uartbb_print(char *message)
{
	while(*message)
	{
		uartbb_send((unsigned int)*message);
		message++;
	}
}
/*----------------------------------------------------------------------------*/
