/*----------------------------------------------------------------------------*/
/**
 * - for accessing mini UART
 *   = part of auxiliary peripheral (along with 2 SPI master)
**/
/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define AUX_BASE (PMAP_BASE|AUX_OFFSET)
#define UART_BASE (PMAP_BASE|UART_OFFSET)
/*----------------------------------------------------------------------------*/
#include "uart.h"
/*----------------------------------------------------------------------------*/
#define AUX_IRQ 0x00
#define AUX_ENABLES 0x01
/*----------------------------------------------------------------------------*/
#define UART_AUX_ENABLE 0x01
/*----------------------------------------------------------------------------*/
#define UART_IO_REG   0x00
#define UART_IER_REG  0x01
#define UART_IIR_REG  0x02
#define UART_LCR_REG  0x03
#define UART_MCR_REG  0x04
#define UART_LSR_REG  0x05
#define UART_MSR_REG  0x06
#define UART_SCRATCH  0x07
#define UART_CNTL_REG 0x08
#define UART_STAT_REG 0x09
#define UART_BAUD_REG 0x0A
/*----------------------------------------------------------------------------*/
volatile unsigned int *uart;
/*----------------------------------------------------------------------------*/
#include "gpio.h"
/*----------------------------------------------------------------------------*/
void uart_init(int baudrate)
{
	uart = (unsigned int*) AUX_BASE;
	uart[AUX_ENABLES] |= UART_AUX_ENABLE;
	uart = (unsigned int*) UART_BASE;
	uart[UART_LCR_REG]  = 0x03; /** 8-bit data (errata in manual 0x01) */
	uart[UART_MCR_REG]  = 0x00;
	uart[UART_IER_REG]  = 0x00; /** no need interrupt */
	/** check requested baudrate **/
	switch (baudrate)
	{
		case UART_BAUD_115200:
		case UART_BAUD_9600:
			break;
		default:
			baudrate = UART_BAUD_DEFAULT;
	}
	/** baudrate count = ((sys_clk/baudrate)/8)-1 */
	uart[UART_BAUD_REG] = baudrate; /** 16-bit baudrate counter */
	/* setup pins! */
	gpio_config(UART_TXD_GPIO,GPIO_ALTF5);
	gpio_config(UART_RXD_GPIO,GPIO_ALTF5);
	/* disable pull-down default on tx/rx pins */
	gpio_pull(UART_TXD_GPIO,GPIO_PULL_NONE);
	gpio_pull(UART_RXD_GPIO,GPIO_PULL_NONE);
	/** ready to go? */
	uart[UART_IIR_REG]  = 0xC6; /** clear TX/RX FIFO */
	uart[UART_CNTL_REG] = 0x03; /** enable TX/RX */
/*
	uart[UART_IIR_REG]  |= 0x06;
	uart[UART_CNTL_REG] |= 0x03;
*/
}
/*----------------------------------------------------------------------------*/
#define UART_TXFIFO_EMPTY 0x20
#define UART_RXFIFO_AVAIL 0x01
/*----------------------------------------------------------------------------*/
void uart_send(unsigned int data)
{
	while(!(uart[UART_LSR_REG]&UART_TXFIFO_EMPTY));
	uart[UART_IO_REG] = data;
}
/*----------------------------------------------------------------------------*/
unsigned int uart_incoming(void)
{
	return uart[UART_LSR_REG]&UART_RXFIFO_AVAIL;
}
/*----------------------------------------------------------------------------*/
unsigned int uart_read(void)
{
	while(!(uart[UART_LSR_REG]&UART_RXFIFO_AVAIL));
	return uart[UART_IO_REG]&0xFF;
}
/*----------------------------------------------------------------------------*/
void uart_print_hex(unsigned int value)
{
	int loop;
	unsigned int test,high;
	uart_send('0');
	uart_send('x');
	for(loop=3;loop>=0;loop--)
	{
		test = (value >> (loop*8)) & 0xff;
		high = (test & 0xf0) >> 4;
		if(high>9) high = (high-10)+0x41;
		else high += 0x30;
		uart_send(high);
		high = (test & 0x0f);
		if(high>9) high = (high-10)+0x41;
		else high += 0x30;
		uart_send(high);
	}
}
/*----------------------------------------------------------------------------*/
void uart_print(char *message)
{
	while(*message)
	{
		uart_send((unsigned int)*message);
		message++;
	}
}
/*----------------------------------------------------------------------------*/
