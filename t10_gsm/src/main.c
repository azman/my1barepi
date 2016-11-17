/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "uartbb.h"
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 80
#define TIMER_10S 10000000
#define TIMER_100MS 100000
#define GSM_WAIT_DELAY 1000
#define GSM_OK "\r\nOK\r\n"
#define GSM_OK_SIZE 6
/*----------------------------------------------------------------------------*/
void gsm_command(char* message)
{
	uart_print(message);
	uart_send(0x0d); /** CR only */
}
/*----------------------------------------------------------------------------*/
int gsm_patience(char* message, int size)
{
	int count = 0, check = 0;
	do {
		message[count] = uart_read();
		count++;
		if (count==size-1) break;
		if (!check)
		{
			if (count>1&&message[0]==0x0d&&message[1]==0x0a)
				check = 1;
		}
		else
		{
			if (count>3&&message[count-2]==0x0d&&message[count-1]==0x0a)
				break;
		}
	}
	while (1);
	message[count] = 0x0;
	return count;
}
/*----------------------------------------------------------------------------*/
int gsm_replies(char* message, int size)
{
	int count = 0;
	do {
		message[count] = uart_read();
		count++;
		if (count==size-1) break;
		if (!uart_incoming())
			timer_wait(GSM_WAIT_DELAY); /** allow delay between bytes */
	}
	while (uart_incoming());
	message[count] = 0x0;
	return count;
}
/*----------------------------------------------------------------------------*/
int gsm_checkit(char* message, int size)
{
	timer_wait(TIMER_100MS); /** allow 100ms before checking response */
	if (!uart_incoming()) return 0;
	return gsm_replies(message,size);
}
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
void do_debug(char* buffer, int count)
{
	uartbb_print("INVALID RESPONSE.\n");
	uartbb_print("[DEBUG]\n{");
	uartbb_print(buffer);
	uartbb_print("}\n[DONE]<");
	uartbb_print_hex(count);
	uartbb_print(">\n");
}
/*----------------------------------------------------------------------------*/
#define CALL_GPIO 2
#define MESG_GPIO 3
/*----------------------------------------------------------------------------*/
void main(void)
{
	int count = 0;
	char buffer[BUFF_SIZE];
	/** initialize stuffs */
	gpio_init();
	gpio_config(CALL_GPIO,GPIO_INPUT);
	gpio_setevent(CALL_GPIO,GPIO_EVENT_AEDGF);
	gpio_config(MESG_GPIO,GPIO_INPUT);
	gpio_setevent(MESG_GPIO,GPIO_EVENT_AEDGF);
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	uartbb_init(UARTBB_RX_DEFAULT,UARTBB_TX_DEFAULT);
	/** send out the word! */
	uartbb_print("\n\n");
	uartbb_print("YAY! It is alive!\n");
	/** wait for modem to settle down? */
	uartbb_print("Press button on the TC35 GSM board now...\n");
	timer_wait(TIMER_10S);
	/** flush fifo buffers */
	while(uart_incoming()) uart_read();
	while(1)
	{
		/* wait for modem response, disable echo! */
		uartbb_print("Sending 'ATE'... ");
		gsm_command("ATE");
		count = gsm_checkit(buffer,BUFF_SIZE);
		if (count==0)
		{
			uartbb_print("no response.\n");
		}
		else
		{
			if (!strncmp(buffer,GSM_OK,GSM_OK_SIZE))
			{
				uartbb_print("OK.\n");
				break;
			}
			else if (!strncmp(buffer,"ATE\r\r\nOK",8))
			{
				uartbb_print("OK.\n");
				break;
			}
			else do_debug(buffer,count);
		}
		timer_wait(3000000); /* wait between retries */
	}
	while(1)
	{
		/* check if sim card is in? */
		uartbb_print("Checking SIM status... ");
		gsm_command("AT+CPIN?");
		//count = gsm_replies(buffer,BUFF_SIZE);
		count = gsm_patience(buffer,BUFF_SIZE);
		if (count==0)
		{
			uartbb_print("should not get here!\n");
		}
		else
		{
			if (!strncmp(buffer,"\r\n+CPIN: READY\r\n",16))
			{
				uartbb_print("SIM ready.\n");
				gsm_replies(buffer,BUFF_SIZE); /*  get OK! */
				break;
			}
			else if (!strncmp(buffer,"\r\nERROR\r\n",9))
			{
				uartbb_print("missing SIM?\n");
			}
			else do_debug(buffer,count);
		}
		timer_wait(3000000); /* wait between retries */
	}
	/** do the thing... */
	while(1)
	{
		uartbb_print("Setting SMS Text Mode... ");
		gsm_command("AT+CMGF=1");
		count = gsm_replies(buffer,BUFF_SIZE);
		if (count==0)
		{
			uartbb_print("no response?\n");
		}
		else
		{
			if (!strncmp(buffer,GSM_OK,GSM_OK_SIZE))
			{
				uartbb_print("done!\n");
				break;
			}
			else do_debug(buffer,count);
		}
		timer_wait(3000000); /* wait between retries */
	}
	/** clear invalid gpio events - just in case */
	gpio_rstevent(CALL_GPIO);
	gpio_rstevent(MESG_GPIO);
	/** main loop */
	while(1)
	{
		if (gpio_chkevent(MESG_GPIO))
		{
			/** send the thing... */
			uartbb_print("Sending SMS... ");
			gsm_command("AT+CMGS=\"+601110967797\"");
			while(uart_read()!='>');
			uart_print("HELLO, WORLD!");
			uart_send(0x1a); // ctrl+z
			count = gsm_replies(buffer,BUFF_SIZE);
			do_debug(buffer,count);
			gpio_rstevent(MESG_GPIO);
			gpio_rstevent(CALL_GPIO); /* cannot request call while messaging */
			continue;
		}
		if (gpio_chkevent(CALL_GPIO))
		{
			/** make a call... */
			uartbb_print("Calling... ");
			gsm_command("ATD+601110967797;");
			uartbb_print("\n=> ");
			timer_wait(3000000); /* wait 3s before hanging up! */
			uartbb_print("\nHanging up... ");
			gsm_command("ATH");
			count = gsm_replies(buffer,BUFF_SIZE);
			if (strncmp(buffer,GSM_OK,GSM_OK_SIZE)) /* something is wrong? */
				do_debug(buffer,count);
			else uartbb_print("done!\n");
			gpio_rstevent(CALL_GPIO);
			gpio_rstevent(MESG_GPIO); /* cannot do messaging while calling */
			continue;
		}
	}
}
/*----------------------------------------------------------------------------*/
