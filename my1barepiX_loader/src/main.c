/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
#define ARM_INIT 0x8000
/*----------------------------------------------------------------------------*/
#define XMODEM_HEAD_SIZE 3
#define XMODEM_PACK_SIZE 128
#define XMODEM_CSUM_SIZE 1
#define XMODEM_BUFF_SIZE 132
/*----------------------------------------------------------------------------*/
#define XMODEM_FILE_TIMEOUT 10000000
#define XMODEM_PACK_TIMEOUT 7000000
/*----------------------------------------------------------------------------*/
#define ASCII_SOH 0x01
#define ASCII_ACK 0x06
#define ASCII_NAK 0x15
#define ASCII_EOT 0x04
#define ASCII_CR  0x0D
/*----------------------------------------------------------------------------*/
void exec_this(unsigned int); /** decribed in assembly */
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int prev, test, crcc, loop, index, check;
	unsigned char buff[XMODEM_BUFF_SIZE];
	unsigned char *pbase;
	char mesg[] = "\nMY1 R-PI LOADER\n";
	gpio_init();
	uart_init(UART_BAUD_9600);
	timer_init();
	/** send out the word! */
	uart_print(mesg);
	/** do the thing... */
	index = 0; check = 1; pbase = (unsigned char*) ARM_INIT;
	prev = timer_read();
	while(1)
	{
		/** implement timeout?  */
		test = timer_read();
		if((test-prev)>=XMODEM_FILE_TIMEOUT)
		{
			uart_send(ASCII_NAK);
			prev = test;
			/* should we reset all counts? */
		}
		if(!uart_incoming()) continue;
		buff[index] = uart_read();
		test = timer_read();
		switch(index)
		{
			case 0:
			{
				if(buff[index]==ASCII_EOT) /** maybe allow EOF as well? */
				{
					uart_send(ASCII_ACK);
					uart_print("\nXMODEM TRANSFER COMPLETE! EXECUTING...\n");
					exec_this(ARM_INIT);
					/* WILL NOT PASS THIS! */
				}
				if(buff[index]==ASCII_SOH)
				{
					crcc = buff[index];
					index++;
				}
				else if(buff[index]==ASCII_CR)
				{
					uart_print(mesg);
					prev = timer_read();
					continue; /* is this valid?? */
				}
				else uart_send(ASCII_NAK);
				break;
			}
			case 1:
			{
				if(buff[index]==check)
				{
					crcc += buff[index];
					index++;
				}
				else
				{
					uart_send(ASCII_NAK);
					index = 0;
				}
				break;
			}
			case 2:
			{
				if(buff[index]==(0xFF-check))
				{
					crcc += buff[index];
					index++;
				}
				else
				{
					uart_send(ASCII_NAK);
					index = 0;
				}
				break;
			}
			case (XMODEM_BUFF_SIZE-1):
			{
				crcc &= 0xFF;
				if(buff[index]==crcc)
				{
					for(loop=0;loop<XMODEM_PACK_SIZE;loop++)
					{
						*pbase = buff[loop+XMODEM_HEAD_SIZE];
						pbase++;
					}
					uart_send(ASCII_ACK);
					check++;
					if(check>255) check = 1;
				}
				else uart_send(ASCII_NAK);
				index=0;
				break;
			}
			default:
			{
				crcc += buff[index++];
				break;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
