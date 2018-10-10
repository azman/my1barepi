/*----------------------------------------------------------------------------*/
#include "bluez.h"
#include "timer.h"
#include "uart.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
#define BLUEZ_WAIT_DELAY (3*TIMER_S)
/*----------------------------------------------------------------------------*/
int bt_timeout(int delay)
{
	unsigned int init = timer_read(), timeout = 1;
	while(timer_read()-init<delay)
		if (uart_incoming()) { timeout = 0; break; }
	return timeout;
}
/*----------------------------------------------------------------------------*/
int bt_replies(btmodule_t* btinfo)
{
	int count = 0;
	btinfo->status = 0;
	if (!bt_timeout(BLUEZ_WAIT_DELAY))
	{
		while (uart_incoming())
		{
			btinfo->buff[count] = uart_read();
			count++;
			if (count==BT_BUFF_SIZE-1)
			{
				btinfo->status = -1;
				break;
			}
		}
		btinfo->buff[count] = 0x0;
		if (count>1&&btinfo->buff[0]=='O'&&btinfo->buff[1]=='K')
			btinfo->status = count;
	}
	return btinfo->status;
}
/*----------------------------------------------------------------------------*/
int bt_cmdwait(btmodule_t* btinfo)
{
	int count = 0;
	btinfo->status = 0;
	while (!uart_incoming());
	while (uart_incoming())
	{
		btinfo->buff[count] = uart_read();
		count++;
		if (count==BT_BUFF_SIZE-1)
		{
			btinfo->status = -1;
			break;
		}
	}
	btinfo->buff[count] = 0x0;
	if (count>1&&btinfo->buff[0]=='O'&&btinfo->buff[1]=='K')
		btinfo->status = count;
	return btinfo->status;
}
/*----------------------------------------------------------------------------*/
void bt_init(btmodule_t* btinfo)
{
	int loop;
	/* try to communicate */
	uart_print("AT");
	timer_wait(TIMER_S);
	if (bt_replies(btinfo)<=0) return;
	/* get version */
	uart_print("AT+VERSION");
	timer_wait(TIMER_S);
	if (bt_replies(btinfo)<=0) return;
	/* save version name */
	loop = 0;
	while (btinfo->buff[loop+2])
	{
		btinfo->vers[loop] = btinfo->buff[loop+2];
		loop++;
	}
	btinfo->vers[loop] = 0x0;
	/* set name */
	uart_print("AT+NAME");
	uart_print(btinfo->name);
	timer_wait(TIMER_S);
	if (bt_replies(btinfo)<=0) return;
	/* set pin */
	uart_print("AT+PIN");
	uart_print(btinfo->cpin);
	timer_wait(TIMER_S);
	if (bt_replies(btinfo)<=0) return;
	/* set baud 9600... always!*/
	uart_print("AT+BAUD4");
	timer_wait(TIMER_S);
	if (bt_replies(btinfo)<=0) return;
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
}
/*----------------------------------------------------------------------------*/
void bt_send(unsigned int data)
{
	uart_send(data);
}
/*----------------------------------------------------------------------------*/
void bt_print(char *message)
{
	uart_print(message);
}
/*----------------------------------------------------------------------------*/
unsigned int bt_scan(void)
{
	return uart_incoming();
}
/*----------------------------------------------------------------------------*/
unsigned int bt_read(btmodule_t* btinfo)
{
	unsigned int test = uart_read();
	if (btinfo)
		btinfo->buff[btinfo->bbsize++] = (unsigned char) test;
	return test;
}
/*----------------------------------------------------------------------------*/
void bt_purge(void)
{
	while(uart_incoming()) uart_read();
}
/*----------------------------------------------------------------------------*/
void bt_print_hexbyte(unsigned char byte)
{
	bt_send(byte2hex(byte,1,1));
	bt_send(byte2hex(byte,0,1));
}
/*----------------------------------------------------------------------------*/
void bt_print_hexuint(unsigned int dwrd)
{
	int loop, pass = 32;
	unsigned int temp;
	for (loop=0;loop<4;loop++)
	{
		pass -= 8;
		temp = dwrd;
		temp >>= pass;
		temp &= 0xff;
		bt_print_hexbyte((unsigned char)temp);
	}
}
/*----------------------------------------------------------------------------*/
void bt_print_int(int value)
{
	char buff[32];
	int2str(buff,value);
	bt_print(buff);
}
/*----------------------------------------------------------------------------*/
