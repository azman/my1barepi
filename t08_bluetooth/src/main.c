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
void main(void)
{
	btmodule_t btdev;
	char *pbuf, *ptmp, buff[32], copy[BT_BUFF_SIZE];
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
		debug_print("Initializing HC-06... ");
		bt_init(&btdev);
		if (btdev.status>0) break;
		debug_print("fail!\n");
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
					strncpy(copy,(char*)btdev.buff,BT_BUFF_SIZE);
					pbuf = copy;
					trimws(pbuf,1);
					str2upper(pbuf);
/*
					int loop = 0;
					while ((ptmp=strword(&pbuf," \n\r\t")))
					{
						loop++;
						bt_print("[Found] Word#");
						int2str(buff,loop);
						bt_print(buff);
						bt_print(": '");
						bt_print(ptmp);
						bt_print("'\n");
					}
*/
					ptmp = strword(&pbuf," \n\r\t");
					if (!ptmp)
					{
						bt_print("[ERROR] No word found? {");
						bt_print((char*)btdev.buff);
						bt_print("}\n");
					}
					else if (strncmp(ptmp,"GPIO",BT_BUFF_SIZE)==0)
					{
						do
						{
							int test;
							/* get gpio select */
							ptmp = strword(&pbuf," \n\r\t");
							if (!ptmp)
							{
								bt_print("[ERROR] No gpio selected!\n");
								break;
							}
							test = str2int(ptmp);
							if (test<2||test>27)
							{
								bt_print("[ERROR] Invalid gpio selected! {");
								bt_print(ptmp);
								bt_print("}\n");
								break;
							}
							/* get gpio task */
							ptmp = strword(&pbuf," \n\r\t");
							if (!ptmp)
							{
								bt_print("[ERROR] No task for gpio command!\n");
								break;
							}
							else if (strncmp(ptmp,"CONFIG",BT_BUFF_SIZE)==0)
							{
								ptmp = strword(&pbuf," \n\r\t");
								if (!ptmp)
								{
									bt_print("[ERROR] No option for ");
									bt_print("gpio config!\n");
									break;
								}
								else if (strncmp(ptmp,"IN",BT_BUFF_SIZE)==0)
								{
									gpio_config(test,GPIO_INPUT);
									bt_print("[GPIO] GPIO");
									int2str(buff,test);
									bt_print(buff);
									bt_print(" configured as input!\n");
								}
								else if (strncmp(ptmp,"OUT",BT_BUFF_SIZE)==0)
								{
									gpio_config(test,GPIO_OUTPUT);
									bt_print("[GPIO] GPIO");
									int2str(buff,test);
									bt_print(buff);
									bt_print(" configured as output!\n");
								}
								else
								{
									bt_print("[ERROR] Invalid option for ");
									bt_print("gpio config!\n");
									break;
								}
							}
							else if (strncmp(ptmp,"SET",BT_BUFF_SIZE)==0)
							{
								gpio_set(test);
								bt_print("[GPIO] GPIO");
								int2str(buff,test);
								bt_print(buff);
								bt_print(" set to logic HI!\n");
							}
							else if (strncmp(ptmp,"CLR",BT_BUFF_SIZE)==0)
							{
								gpio_clr(test);
								bt_print("[GPIO] GPIO");
								int2str(buff,test);
								bt_print(buff);
								bt_print(" set to logic LO!\n");
							}
							else if (strncmp(ptmp,"STATUS",BT_BUFF_SIZE)==0)
							{
								bt_print("[GPIO] GPIO");
								int2str(buff,test);
								bt_print(buff);
								bt_print(" status is at logic ");
								if (gpio_read(test)) bt_print("HI");
								else bt_print("LO");
								bt_print("!\n");
							}
							else
							{
								bt_print("[ERROR] Invalid gpio task! {");
								bt_print(ptmp);
								bt_print("}\n");
							}
						}
						while(0);
					}
					else
					{
						bt_print("[ERROR] Unknown command! {");
						bt_print(ptmp);
						bt_print("}\n");
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
