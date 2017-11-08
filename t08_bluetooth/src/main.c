/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "uartbb.h"
#include "bluez.h"
#include "string.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
void debug_char(char chk)
{
	uartbb_send(chk);
}
/*----------------------------------------------------------------------------*/
void debug_hexbyte(unsigned char byte)
{
	unsigned char temp = (byte & 0xf0) >> 4;
	if(temp>9) temp = (temp-10)+0x41;
	else temp += 0x30;
	debug_char(temp);
	temp = (byte & 0x0f);
	if(temp>9) temp = (temp-10)+0x41;
	else temp += 0x30;
	debug_char(temp);
}
/*----------------------------------------------------------------------------*/
void debug_print(char *msg)
{
	uartbb_print(msg);
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
void main(void)
{
	btmodule_t btdev;
	char *pbuf, *ptmp, buff[32];
	int test, loop;
	/** initialize basics */
	gpio_init();
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	uartbb_init(UARTBB_RX_DEFAULT,UARTBB_TX_DEFAULT);
	/** announce our presence */
	debug_print("Testing bluetooth!\n\n");
	/** initialize bt structure */
	btdev.status = 0;
	btdev.bbsize = 0;
	strncpy(btdev.name,"my1bluex",BT_NAME_BUFF);
	strncpy(btdev.cpin,"1234",BT_CPIN_BUFF);
	btdev.vers[0] = 0x0;
	/** check hc-06 interface */
	while (1)
	{
		bt_init(&btdev);
#if 0
		do
		{
			int loop;
			/* try to communicate */
			debug_print("Sending 'AT'... ");
			uart_print("AT");
			timer_wait(TIMER_S);
			if (bt_replies(&btdev)>0)
			{
				debug_print("'");
				debug_print((char*)btdev.buff);
				debug_print("'\n");
			}
			else
			{
				debug_print("no response!\n");
				break;
			}
			/* get version */
			debug_print("Getting version... ");
			uart_print("AT+VERSION");
			timer_wait(TIMER_S);
			if (bt_replies(&btdev)>0)
			{
				loop = 0;
				while (btdev.buff[loop+2])
				{
					btdev.vers[loop] = btdev.buff[loop+2];
					loop++;
				}
				btdev.vers[loop] = 0x0;
				debug_print("'");
				debug_print(btdev.vers);
				debug_print("'\n");
			}
			else
			{
				debug_print("no response!\n");
				break;
			}
			/* set name */
			debug_print("Setting name... ");
			uart_print("AT+NAME");
			uart_print(btdev.name);
			timer_wait(TIMER_S);
			if (bt_cmdwait(&btdev)>0)
			{
				debug_print("'");
				debug_print((char*)btdev.buff);
				debug_print("'\n");
			}
			else
			{
				debug_print("no response!\n");
				break;
			}
			/* set pin */
			debug_print("Setting pin... ");
			uart_print("AT+PIN");
			uart_print(btdev.cpin);
			timer_wait(TIMER_S);
			if (bt_cmdwait(&btdev)>0)
			{
				debug_print("'");
				debug_print((char*)btdev.buff);
				debug_print("'\n");
			}
			else
			{
				debug_print("no response!\n");
				break;
			}
			/* set baud 9600... always!*/
			debug_print("Setting baud... ");
			uart_print("AT+BAUD4");
			timer_wait(TIMER_S);
			if (bt_cmdwait(&btdev)>0)
			{
				debug_print("'");
				debug_print((char*)btdev.buff);
				debug_print("'\n");
			}
			else
			{
				debug_print("no response!\n");
				break;
			}
			timer_wait(TIMER_S);
		}
		while(0);
#endif
		if (btdev.status>0) break;
		timer_wait(TIMER_S);
	}
	debug_print("Module ready!\n");
	/** do the thing... */
	btdev.bbsize = 0;
	while (1)
	{
		if (bt_scan())
		{
			if (btdev.bbsize<BT_BUFF_SIZE-1)
			{
				bt_read(&btdev);
				/* process string if a newline is detected! */
				if (btdev.bbsize>0&&btdev.buff[btdev.bbsize-1]=='\n')
				{
					btdev.buff[btdev.bbsize++] = 0x0;
					pbuf = (char*) btdev.buff;
					trimws(pbuf,1);
					str2upper(pbuf);
					loop = 0;
					while(pbuf)
					{
						ptmp = strword(pbuf," \n\r\t",&test);
						if (test>0)
						{
							loop++;
							bt_print("[Found] Word#");
							int2str(buff,loop);
							bt_print(buff);
							bt_print(": '");
							bt_print(pbuf);
							bt_print("'\n");
						}
						pbuf = ptmp;
					}
					btdev.bbsize = 0;
				}
			}
			else
			{
				bt_purge();
				/* should be paired by now... send word! */
				bt_print("[ERROR] Buffer overflow! Input purged!\n");
				btdev.bbsize = 0;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
