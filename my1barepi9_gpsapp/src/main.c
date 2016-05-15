/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "uartbb.h"
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 128
/*----------------------------------------------------------------------------*/
int strncmp(char* src1, char* src2, int size)
{
	int loop, stat = 0;

	/* strncmp has a size limit */
	for (loop=0;loop<size;loop++)
	{
		/* check if null comes first */
		if (!(*src1))
		{
			if ((*src2)) stat = -1; /* src2 longer string */
			/* else? stop at same time => equal! */
			break;
		}
		if (!(*src2))
		{
			if ((*src1)) stat = 1; /* src1 longer string */
			/* else? stop at same time => equal! */
			break;
		}
		/* actual strcmp! */
		if ((unsigned char)(*src1)>(unsigned char)(*src2))
		{
			stat = 1; break;
		}
		if ((unsigned char)(*src1)<(unsigned char)(*src2))
		{
			stat = -1; break;
		}
		src1++; src2++;
	}
	return stat;
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	int size = 0;
	char buff[BUFF_SIZE];
	/** initialize stuffs */
	gpio_init();
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	uartbb_init(UARTBB_RX_DEFAULT,UARTBB_TX_DEFAULT);
	/** send out the word! */
	uartbb_print("\n\n");
	uartbb_print("YAY! It is alive!\n");
	/** do the thing... */
	while(1)
	{
		buff[size++] = (unsigned char) uart_read(); /** blocking function */
		if (buff[size-1]=='\n') /** check for 0x0a - process this! */
		{
			buff[size] = 0x0;
			size = 0;
			if (buff[0]!='$') /** not a valid nmea if no '$'? */
				continue;
			/** process */
			if (!strncmp(buff,"$GPGLL",6))
			{
				uartbb_print("Found: ");
				uartbb_print(buff);
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
