/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "interrupt.h"
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
/** gpio 19 */
#define RESET_PIN 19
#define CHECK_PIN 26
#define DUMMY_PIN 13
/*----------------------------------------------------------------------------*/
typedef void (*ihandler_t)(void);
/*----------------------------------------------------------------------------*/
/** decribed in assembly */
extern void exec_this(unsigned int);
extern void reset_bootload(void);
extern void set_user_irqh(ihandler_t);
extern void run_user_irqh(void);
/*----------------------------------------------------------------------------*/
void irq_handler(void)
{
	/* how do i allow user interrupt handler?? */
	if(gpio_chkevent(RESET_PIN))
	{
		gpio_rstevent(RESET_PIN);
		reset_bootload();
	}
	run_user_irqh();
}
/*----------------------------------------------------------------------------*/
void chk_handler(void)
{
	if(gpio_chkevent(DUMMY_PIN))
	{
		gpio_rstevent(DUMMY_PIN);
		gpio_toggle(CHECK_PIN);
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int prev, test, crcc, loop, index, check;
	unsigned char buff[XMODEM_BUFF_SIZE];
	unsigned char *pbase;
	char mesg[] = "\nMY1 R-PI LOADER\n";
	uart_init(UART_BAUD_115200);
	timer_init();
	/** send out the word! */
	uart_print(mesg);
	/** do the thing... */
	index = 0; check = 1; pbase = (unsigned char*) ARM_INIT;
	prev = timer_read();
	/* setup interrpt */
	gpio_config(CHECK_PIN,GPIO_OUTPUT);
	gpio_write(CHECK_PIN,1);
	set_user_irqh(chk_handler);
	interrupt_init();
	gpio_config(RESET_PIN,GPIO_INPUT);
	gpio_pull(RESET_PIN,GPIO_PULL_UP);
	gpio_rstevent(RESET_PIN);
	gpio_setevent(RESET_PIN,GPIO_EVENT_AEDGF);
	gpio_config(DUMMY_PIN,GPIO_INPUT);
	gpio_pull(DUMMY_PIN,GPIO_PULL_UP);
	gpio_rstevent(DUMMY_PIN);
	gpio_setevent(DUMMY_PIN,GPIO_EVENT_AEDGF);
	enable_irq();
	interrupt_enable(INTR_IRQSET2,INTR_PEND2_GPIOS);
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
