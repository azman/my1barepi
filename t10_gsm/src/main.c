/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "uartbb.h"
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 80
#define GSM_WAIT_RETRY (3*TIMER_S)
#define GSM_WAIT_CHECK (500*TIMER_MS)
#define GSM_WAIT_DELAY (1*TIMER_MS)
#define GSM_OK "\r\nOK\r\n"
#define GSM_OK_SIZE 6
#define CALL_RING_DELAY (15*TIMER_S)
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
void gsm_command(char* message)
{
	uart_print(message);
	uart_send(0x0d); /** CR only */
}
/*----------------------------------------------------------------------------*/
int gsm_patience(char* message, int size)
{
	/** basically, wait until we get GSM_OK */
	int count = 0;
	do {
		message[count] = uart_read();
		count++;
		if (count==size-1) break;
		else if (count>=GSM_OK_SIZE)
		{
			if (!strncmp(&message[count-GSM_OK_SIZE],GSM_OK,GSM_OK_SIZE))
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
	timer_wait(GSM_WAIT_CHECK); /** allow 100ms before checking response */
	if (!uart_incoming()) return 0;
	return gsm_replies(message,size);
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
#define MESG_GPIO 5
#define CALL_GPIO 6
#define HANG_GPIO 13
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int initc;
	int count = 0, check;
	char buffer[BUFF_SIZE];
	/** initialize stuffs */
	gpio_init();
	gpio_config(MESG_GPIO,GPIO_INPUT);
	gpio_setevent(MESG_GPIO,GPIO_EVENT_AEDGF);
	gpio_config(CALL_GPIO,GPIO_INPUT);
	gpio_setevent(CALL_GPIO,GPIO_EVENT_AEDGF);
	gpio_config(HANG_GPIO,GPIO_INPUT);
	gpio_setevent(HANG_GPIO,GPIO_EVENT_AEDGF);
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	uartbb_init(UARTBB_RX_DEFAULT,UARTBB_TX_DEFAULT);
	/** send out the word! */
	uartbb_print("\n\n");
	uartbb_print("YAY! It is alive!\n");
	/** wait for modem to settle down? */
	//uartbb_print("Press button on the TC35 GSM board now...\n");
	//timer_wait(10*TIMER_S);
	/** flush fifo buffers */
	//while(uart_incoming()) uart_read();
	/* looking for modem response */
	while(1)
	{
		uartbb_print("Sending 'AT'... ");
		gsm_command("AT");
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
			else do_debug(buffer,count);
		}
		timer_wait(GSM_WAIT_RETRY); /* wait between retries */
	}
	/* disable echo */
	while(1)
	{
		uartbb_print("Sending 'ATE 0'... ");
		gsm_command("ATE 0");
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
			else do_debug(buffer,count);
		}
		timer_wait(GSM_WAIT_RETRY);
	}
	/* check if sim card is in */
	while(1)
	{
		uartbb_print("Checking SIM status... ");
		gsm_command("AT+CPIN?");
		count = gsm_replies(buffer,BUFF_SIZE);
		//count = gsm_patience(buffer,BUFF_SIZE);
		if (count==0)
		{
			uartbb_print("should not get here!\n");
		}
		else
		{
			/* trim prefixed /r/n, some gsm module do not send these */
			check = 0;
			while(buffer[check]=='\r'||buffer[check]=='\n') check++;
			if (!strncmp(&buffer[check],"+CPIN: READY",12))
			{
				uartbb_print("SIM ready.\n");
				break;
			}
			else if (!strncmp(buffer,"ERROR",5))
			{
				uartbb_print("missing SIM?\n");
			}
			else do_debug(buffer,count);
		}
		timer_wait(GSM_WAIT_RETRY);
	}
	/* setting text mode for sending sms */
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
		timer_wait(GSM_WAIT_RETRY);
	}
	/** clear invalid gpio events - just in case */
	gpio_rstevent(MESG_GPIO);
	gpio_rstevent(CALL_GPIO);
	gpio_rstevent(HANG_GPIO);
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
			gpio_rstevent(HANG_GPIO);
			continue;
		}
		if (gpio_chkevent(CALL_GPIO))
		{
			gpio_rstevent(HANG_GPIO);
			/** make a call... */
			uartbb_print("Calling... ");
			gsm_command("ATD+601110967797;");
			uartbb_print("\n=> ");
			initc = timer_read();
			while(timer_read()-initc<CALL_RING_DELAY)
			{
				if (gpio_chkevent(HANG_GPIO)) break;
			}
			uartbb_print("\nHanging up... ");
			gsm_command("ATH");
			count = gsm_replies(buffer,BUFF_SIZE);
			if (strncmp(buffer,GSM_OK,GSM_OK_SIZE)) /* something is wrong? */
				do_debug(buffer,count);
			else uartbb_print("done!\n");
			gpio_rstevent(CALL_GPIO);
			gpio_rstevent(HANG_GPIO);
			gpio_rstevent(MESG_GPIO); /* cannot do messaging while calling */
			continue;
		}
	}
}
/*----------------------------------------------------------------------------*/
