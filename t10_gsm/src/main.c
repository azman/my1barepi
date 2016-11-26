/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "uartbb.h"
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 80
#define GSM_WAIT_RETRY (3*TIMER_S)
#define GSM_WAIT_CHECK (2*TIMER_S)
#define GSM_WAIT_DELAY (1000*TIMER_MS)
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
int gsm_checkok(char* message, int count)
{
	int ok_found = 0;
	if (count>=GSM_OK_SIZE)
		if (!strncmp(&message[count-GSM_OK_SIZE],GSM_OK,GSM_OK_SIZE))
			ok_found = 1;
	return ok_found;
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
int gsm_timeout(int delay)
{
	unsigned int init = timer_read(), time = 1;
	while(timer_read()-init<delay)
		if (uart_incoming()) { time = 0; break; }
	return time;
}
/*----------------------------------------------------------------------------*/
int gsm_replies(char* message, int size)
{
	int count = 0;
	do {
		message[count] = uart_read();
		count++;
		if (count==size-1) break;
		gsm_timeout(GSM_WAIT_DELAY); /** allow delay between bytes */
	}
	while (uart_incoming());
	message[count] = 0x0;
	return count;
}
/*----------------------------------------------------------------------------*/
int gsm_checkit(char* message, int size)
{
	/** allow delay before checking response */
	if (gsm_timeout(GSM_WAIT_CHECK)) return 0;
	return gsm_replies(message,size);
}
/*----------------------------------------------------------------------------*/
char* gsm_trim_prefix(char* message)
{
	/* trim prefixed \r\n, some gsm module do not send these */
	char *pbuff = message;
	while(*pbuff=='\r'||*pbuff=='\n') pbuff++;
	return pbuff;
}
/*----------------------------------------------------------------------------*/
void do_debug(char* buffer, int count)
{
	int loop;
	uartbb_print("INVALID RESPONSE.\n");
	uartbb_print("[DEBUG]\n{");
	for (loop=0;loop<count;loop++)
	{
		unsigned int byte = (unsigned int)(buffer[loop]), temp;
		if (byte<0x20||byte>0x7f)
		{
			uartbb_send('[');
			uartbb_send('0');
			uartbb_send('x');
			temp = (byte & 0xf0) >> 4;
			if(temp>9) temp = (temp-10)+0x41;
			else temp += 0x30;
			uartbb_send(temp);
			temp = (byte & 0x0f);
			if(temp>9) temp = (temp-10)+0x41;
			else temp += 0x30;
			uartbb_send(temp);
			uartbb_send(']');
		}
		else uartbb_send(byte);
	}
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
	int count = 0;
	char buffer[BUFF_SIZE], *pbuff;
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
			if (gsm_checkok(buffer,count))
			{
				uartbb_print("OK.\n");
				break;
			}
			/** else do_debug(buffer,count); */
			else uartbb_print("invalid response.\n");
		}
		timer_wait(GSM_WAIT_RETRY); /* wait between retries */
	}
	/* try to disable echo - tc35 needs this */
	uartbb_print("Sending 'ATE'... ");
	gsm_command("ATE");
	count = gsm_checkit(buffer,BUFF_SIZE);
	if (count==0)
	{
		uartbb_print("no response. Abort!\n");
		while(1);
	}
	else
	{
		if (gsm_checkok(buffer,count)) uartbb_print("OK.\n");
		else do_debug(buffer,count);
	}
	/* on sim908, must send ate 0 */
	uartbb_print("Sending 'ATE 0'... ");
	gsm_command("ATE 0");
	count = gsm_checkit(buffer,BUFF_SIZE);
	if (count==0)
	{
		uartbb_print("no response. Abort!\n");
		while(1);
	}
	else
	{
		pbuff = gsm_trim_prefix(buffer);
		if (gsm_checkok(buffer,count)) uartbb_print("OK.\n");
		else if (!strncmp(pbuff,"ERROR",5)) uartbb_print("ignored.\n");
		else do_debug(buffer,count);
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
			pbuff = gsm_trim_prefix(buffer);
			if (!strncmp(pbuff,"+CPIN: READY",12))
			{
				uartbb_print("SIM ready.\n");
				break;
			}
			else if (!strncmp(pbuff,"ERROR",5))
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
