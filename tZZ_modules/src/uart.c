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
#include "boot.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define AUX_IRQ     (AUX_BASE+0x00)
#define AUX_ENABLES (AUX_BASE+0x04)
/*----------------------------------------------------------------------------*/
#define UART_AUX_ENABLE 0x01
/*----------------------------------------------------------------------------*/
#define UART_IO_REG   (UART_BASE+0x00)
#define UART_IER_REG  (UART_BASE+0x04)
#define UART_IIR_REG  (UART_BASE+0x08)
#define UART_LCR_REG  (UART_BASE+0x0C)
#define UART_MCR_REG  (UART_BASE+0x10)
#define UART_LSR_REG  (UART_BASE+0x14)
#define UART_MSR_REG  (UART_BASE+0x18)
#define UART_SCRATCH  (UART_BASE+0x1C)
#define UART_CNTL_REG (UART_BASE+0x20)
#define UART_STAT_REG (UART_BASE+0x24)
#define UART_BAUD_REG (UART_BASE+0x28)
/*----------------------------------------------------------------------------*/
void uart_init(int baudrate)
{
	unsigned int data = get32(AUX_ENABLES);
	/* enable uart */
	put32(AUX_ENABLES,data|UART_AUX_ENABLE);
	/* configure uart */
	put32(UART_LCR_REG,0x03); /** 8-bit data (errata in manual 0x01) */
	put32(UART_MCR_REG,0x00);
	put32(UART_IER_REG,0x00); /** no need interrupt */
	/** check requested baudrate **/
	switch (baudrate)
	{
		case UART_BAUD_115200:
		case UART_BAUD_57600:
		case UART_BAUD_9600:
			break;
		default:
			baudrate = UART_BAUD_DEFAULT;
	}
	put32(UART_BAUD_REG,baudrate); /** 16-bit baudrate counter */
	/* disable pull-down default on tx/rx pins */
	gpio_pull(UART_TXD_GPIO,GPIO_PULL_NONE);
	gpio_pull(UART_RXD_GPIO,GPIO_PULL_NONE);
	/* setup uart TX1/RX1 at pins 14/15 (ALTF5) */
	gpio_config(UART_TXD_GPIO,GPIO_ALTF5);
	gpio_config(UART_RXD_GPIO,GPIO_ALTF5);
	/** ready to go? */
	put32(UART_IIR_REG,0xC6); /** clear TX/RX FIFO */
	put32(UART_CNTL_REG,0x03); /** enable TX/RX */
}
/*----------------------------------------------------------------------------*/
#define UART_TXFIFO_EMPTY 0x20
#define UART_RXFIFO_AVAIL 0x01
/*----------------------------------------------------------------------------*/
void uart_send(unsigned int data)
{
	while(!(get32(UART_LSR_REG)&UART_TXFIFO_EMPTY));
	put32(UART_IO_REG,data);
}
/*----------------------------------------------------------------------------*/
unsigned int uart_incoming(void)
{
	return get32(UART_LSR_REG)&UART_RXFIFO_AVAIL;
}
/*----------------------------------------------------------------------------*/
unsigned int uart_read(void)
{
	while(!(get32(UART_LSR_REG)&UART_RXFIFO_AVAIL));
	return get32(UART_IO_REG)&0xFF;
}
/*----------------------------------------------------------------------------*/
void uart_purge(void)
{
	while(uart_incoming()) uart_read();
}
/*----------------------------------------------------------------------------*/
void uart_print_hex_byte(unsigned char byte)
{
	unsigned char temp = (byte & 0xf0) >> 4;
	if(temp>9) temp = (temp-10)+0x41;
	else temp += 0x30;
	uart_send(temp);
	temp = (byte & 0x0f);
	if(temp>9) temp = (temp-10)+0x41;
	else temp += 0x30;
	uart_send(temp);
}
/*----------------------------------------------------------------------------*/
void uart_print_hex_uint(unsigned int value)
{
	int loop, temp;
	for(loop=3;loop>=0;loop--)
	{
		temp = (value >> (loop*8)) & 0xff;
		uart_print_hex_byte((unsigned char)temp);
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
