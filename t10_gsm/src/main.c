/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "gsm.h"
#include "string.h"
#include "mailbox.h"
#include "barrier.h"
#include "utils.h"
#include "video.h"
/*----------------------------------------------------------------------------*/
#define PHONENUM "+601110967797"
/*----------------------------------------------------------------------------*/
#define MESG_GPIO 5
#define CALL_GPIO 6
#define HANG_GPIO 13
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 80
#define GSM_WAIT_RETRY (3*TIMER_S)
#define CALL_RING_DELAY (30*TIMER_S)
/*----------------------------------------------------------------------------*/
void debug_hexbyte(unsigned char byte)
{
	video_text_char(byte2hex(byte,1,0));
	video_text_char(byte2hex(byte,0,0));
}
/*----------------------------------------------------------------------------*/
void debug_char(char chk)
{
	video_text_char(chk);
}
/*----------------------------------------------------------------------------*/
void debug_print(char *msg)
{
	video_text_string(msg);
}
/*----------------------------------------------------------------------------*/
void do_debug(char* chkstr, char* buffer, int count)
{
	int loop;
	if (chkstr) debug_print(chkstr);
	debug_print("[DEBUG] {");
	for (loop=0;loop<count;loop++)
	{
		unsigned int byte = (unsigned int)(buffer[loop]);
		if (byte<0x20||byte>0x7f)
		{
			debug_print("[0x");
			debug_hexbyte((unsigned char)byte);
			debug_char(']');
		}
		else debug_char(byte);
	}
	debug_print("}[DONE](0x");
	debug_hexbyte((unsigned char)count); /* count < 255! */
	debug_print(")\n");
}
/*----------------------------------------------------------------------------*/
void do_check(char* buffer, int count)
{
	int check = gsm_replies(buffer,count,0x0);
	if (check) do_debug(0x0,buffer,check);
}
/*----------------------------------------------------------------------------*/
void do_abort(void)
{
	debug_print("ABORT!");
	while(1);
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int initc = 0, okchk;
	int count = 0, reset = 1;
	char buffer[BUFF_SIZE], *pbuff;
	/** initialize stuffs */
	gpio_config(MESG_GPIO,GPIO_INPUT);
	gpio_pull(MESG_GPIO,GPIO_PULL_UP);
	gpio_setevent(MESG_GPIO,GPIO_EVENT_AEDGR);
	gpio_config(CALL_GPIO,GPIO_INPUT);
	gpio_pull(CALL_GPIO,GPIO_PULL_UP);
	gpio_setevent(CALL_GPIO,GPIO_EVENT_AEDGR);
	gpio_config(HANG_GPIO,GPIO_INPUT);
	gpio_pull(HANG_GPIO,GPIO_PULL_UP);
	gpio_setevent(HANG_GPIO,GPIO_EVENT_AEDGR);
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	/** initialize video stuff - mailbox before that! */
	mailbox_init();
	video_init(VIDEO_RES_VGA);
	video_clear();
	/** send out the word! */
	debug_print("\n\nYAY! It is alive!\n");
	/* flush incoming uart buffer */
	uart_purge();
	/* looking for modem response */
	while(1)
	{
		debug_print("Sending 'AT'... ");
		gsm_command("AT");
		count = gsm_replies(buffer,BUFF_SIZE,&okchk);
		if (count==0)
		{
			debug_print("no response.\n");
		}
		else
		{
			if (okchk==GSM_OK_COMPLETE)
			{
				debug_print("OK.\n");
				break;
			}
			else
			{
				/** do_debug("invalid response.\n",buffer,count); */
				debug_print("invalid response.\n");
			}
		}
		timer_wait(GSM_WAIT_RETRY); /* wait between retries */
	}
	/* check if gsm module said something */
	do_check(buffer,BUFF_SIZE);
	/* try to disable echo - tc35 needs this */
	debug_print("Sending 'ATE'... ");
	gsm_command("ATE");
	count = gsm_replies(buffer,BUFF_SIZE,&okchk);
	if (count==0)
	{
		debug_print("no response.\n");
		do_abort();
	}
	else
	{
		if (okchk==GSM_OK_COMPLETE)
		{
			debug_print("OK.\n");
		}
		else
		{
			do_debug("invalid response.\n",buffer,count);
			do_abort();
		}
	}
	do_check(buffer,BUFF_SIZE);
	/* on sim908, must send ate 0 */
	debug_print("Sending 'ATE 0'... ");
	gsm_command("ATE 0");
	count = gsm_replies(buffer,BUFF_SIZE,&okchk);
	if (count==0)
	{
		debug_print("no response.\n");
		do_abort();
	}
	else
	{
		pbuff = gsm_trim_prefix(buffer);
		if (okchk==GSM_OK_COMPLETE)
		{
			debug_print("OK.\n");
		}
		else if (!strncmp(pbuff,"ERROR",5))
		{
			debug_print("ignored.\n");
		}
		else
		{
			do_debug("invalid response.\n",buffer,count);
			do_abort();
		}
	}
	do_check(buffer,BUFF_SIZE);
	/* check if sim card is in */
	debug_print("Checking SIM status... ");
	gsm_command("AT+CPIN?");
	count = gsm_replies(buffer,BUFF_SIZE,&okchk);
	if (count==0)
	{
		debug_print("no response.\n");
		do_abort();
	}
	else
	{
		pbuff = gsm_trim_prefix(buffer);
		if (!strncmp(pbuff,"+CPIN: READY",12))
		{
			debug_print("SIM ready.\n");
		}
		else if (!strncmp(pbuff,"ERROR",5))
		{
			debug_print("missing SIM?\n");
			do_abort();
		}
		else
		{
			do_debug("invalid response.\n",buffer,count);
			do_abort();
		}
	}
	do_check(buffer,BUFF_SIZE);
	/* setting text mode for sending sms */
	debug_print("Setting SMS Text Mode... ");
	gsm_command("AT+CMGF=1");
	count = gsm_replies(buffer,BUFF_SIZE,&okchk);
	if (count==0)
	{
		debug_print("no response.\n");
		do_abort();
	}
	else
	{
		if (okchk==GSM_OK_COMPLETE)
		{
			debug_print("OK.\n");
		}
		else
		{
			do_debug("invalid response.\n",buffer,count);
			do_abort();
		}
	}
	/** clear invalid gpio events - just in case */
	gpio_rstevent(MESG_GPIO);
	gpio_rstevent(CALL_GPIO);
	gpio_rstevent(HANG_GPIO);
	/** main loop */
	while(1)
	{
		if (reset)
		{
			debug_print("GSM Module Ready for Call/SMS!\n");
			reset = 0;
		}
		/* any news? */
		if (uart_incoming())
		{
			count = gsm_replies(buffer,count,0x0);
			if (count)
			{
				do_debug(0x0,buffer,count);
				timer_wait(3*TIMER_S);
				reset = 1;
			}
		}
		/* check requests */
		if (gpio_chkevent(MESG_GPIO))
		{
			/** send the thing... */
			debug_print("Sending SMS to "PHONENUM"... ");
			gsm_command("AT+CMGS=\""PHONENUM"\"");
			while(uart_read()!='>');
			uart_print("HELLO, WORLD!");
			uart_send(0x1a); // ctrl+z
			timer_wait(1*TIMER_S); // got one whitespace char on tc35?
			count = gsm_replies(buffer,BUFF_SIZE,&okchk);
			if (count==0)
			{
				debug_print("no response?\n");
			}
			else if (okchk==GSM_OK_COMPLETE)
			{
				debug_print("OK.\n");
			}
			else
			{
				do_debug("invalid response.\n",buffer,count);
				timer_wait(3*TIMER_S);
				reset = 1;
			}
			gpio_rstevent(MESG_GPIO);
			gpio_rstevent(CALL_GPIO); /* cannot request call while messaging */
			gpio_rstevent(HANG_GPIO);
			continue;
		}
		if (gpio_chkevent(CALL_GPIO))
		{
			gpio_rstevent(HANG_GPIO);
			/** make a call... */
			debug_print("Calling "PHONENUM"... ");
			gsm_command("ATD"PHONENUM";");
			initc = timer_read();
			while(timer_read()-initc<CALL_RING_DELAY)
			{
				if (gpio_chkevent(HANG_GPIO))
				{
					debug_print("\nHanging up... ");
					gsm_command("ATH");
					count = gsm_replies(buffer,BUFF_SIZE,&okchk);
					if (count)
					{
						pbuff = gsm_trim_prefix(buffer);
						if (!strncmp(pbuff,"NO CARRIER",10))
						{
							debug_print("done!\n");
							break;
						}
						else
						{
							do_debug("huh?\n",buffer,count);
						}
					}
					else
					{
						debug_print("\n");
					}
					break;
				}
				if (uart_incoming())
				{
					count = gsm_replies(buffer,BUFF_SIZE,&okchk);
					pbuff = gsm_trim_prefix(buffer);
					if (!strncmp(pbuff,"NO CARRIER",10))
					{
						debug_print("call rejected?\n");
						break;
					}
					else
					{
						timer_wait(3*TIMER_S);
						reset = 1;
					}
				}
			}
			gpio_rstevent(CALL_GPIO);
			gpio_rstevent(HANG_GPIO);
			gpio_rstevent(MESG_GPIO); /* cannot do messaging while calling */
			continue;
		}
	}
}
/*----------------------------------------------------------------------------*/
