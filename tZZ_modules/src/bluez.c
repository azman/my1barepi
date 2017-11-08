/*----------------------------------------------------------------------------*/
#include "bluez.h"
#include "timer.h"
#include "uart.h"
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
