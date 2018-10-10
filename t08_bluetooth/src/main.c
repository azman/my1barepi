/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "uartbb.h"
#include "bluez.h"
#include "string.h"
#include "utils.h"
#include "mailbox.h"
/*----------------------------------------------------------------------------*/
void main(void)
{
	btmodule_t btdev;
	tags_info_t info;
	int temp;
	char *pbuf, *ptmp, copy[BT_BUFF_SIZE];
	/** initialize basics */
	gpio_init();
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	uartbb_init(UARTBB_RX_DEFAULT,UARTBB_TX_DEFAULT);
	/** announce our presence */
	uartbb_print("Testing bluetooth!\n\n");
	/** prepare mailbox interface */
	mailbox_init();
	/** initialize bt structure */
	btdev.status = 0;
	btdev.bbsize = 0;
	strncpy(btdev.name,"my1bluez",BT_NAME_BUFF);
	strncpy(btdev.cpin,"4444",BT_CPIN_BUFF);
	btdev.vers[0] = 0x0;
	/** check hc-06 interface */
	while (1)
	{
		uartbb_print("Initializing HC-06... ");
		bt_init(&btdev);
		if (btdev.status>0) break;
		uartbb_print("fail!\n");
		timer_wait(TIMER_S);
	}
	uartbb_print("Module ready!\n");
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
					temp = trimws(pbuf,1);
					str2upper(pbuf);
#if 0
					{
						/* dummy block for debug! */
						int loop = 0;
						/* show the whole buffer */
						bt_print("[Input] '");
						bt_print(pbuf);
						bt_print("' (");
						bt_print_int(temp);
						bt_print(")\n");
						/* show each token detected */
						while ((ptmp=strword(&pbuf," \n\r\t")))
						{
							loop++;
							bt_print("[Found] Word#");
							bt_print_int(loop);
							bt_print(": '");
							bt_print(ptmp);
							bt_print("'\n");
						}
						/* reset buffer for processing */
						strncpy(copy,(char*)btdev.buff,BT_BUFF_SIZE);
						pbuf = copy;
						trimws(pbuf,1);
						str2upper(pbuf);
					}
#endif
					ptmp = strword(&pbuf," \n\r\t");
					if (!ptmp)
					{
						bt_print("[SIGNAL] I AM ALIVE!\n");
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
							if (test<2||test>27||test==14||test==15)
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
									bt_print_int(test);
									bt_print(" configured as input!\n");
								}
								else if (strncmp(ptmp,"OUT",BT_BUFF_SIZE)==0)
								{
									gpio_config(test,GPIO_OUTPUT);
									bt_print("[GPIO] GPIO");
									bt_print_int(test);
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
								bt_print_int(test);
								bt_print(" set to logic HI!\n");
							}
							else if (strncmp(ptmp,"CLR",BT_BUFF_SIZE)==0)
							{
								gpio_clr(test);
								bt_print("[GPIO] GPIO");
								bt_print_int(test);
								bt_print(" set to logic LO!\n");
							}
							else if (strncmp(ptmp,"STATUS",BT_BUFF_SIZE)==0)
							{
								bt_print("[GPIO] GPIO");
								bt_print_int(test);
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
					else if (strncmp(ptmp,"INFO",BT_BUFF_SIZE)==0)
					{
						do
						{
							/* get info selection */
							ptmp = strword(&pbuf," \n\r\t");
							if (!ptmp)
							{
								bt_print("[ERROR] ");
								bt_print("No selection for info command!\n");
								break;
							}
							else if (strncmp(ptmp,"BOARD",BT_BUFF_SIZE)==0)
							{
								bt_print("[WAIT] ");
								bt_print("Gathering board info... ");
								mailbox_get_board_info(&info);
								bt_print("done.\n");
								bt_print("[INFO] Hardware {0x");
								bt_print_hexuint(info.info_status);
								bt_print("}\n");
								bt_print("==> VideoCore Firmware: 0x");
								bt_print_hexuint(info.vc_revision);
								bt_print("\n");
								bt_print("==> Board Model: 0x");
								bt_print_hexuint(info.board_model);
								bt_print("\n");
								bt_print("==> Board Revision: 0x");
								bt_print_hexuint(info.board_revision);
								bt_print("\n");
								bt_print("==> Board MAC Address: 0x");
								bt_print_hexuint(info.board_mac_addrh);
								bt_print(",0x");
								bt_print_hexuint(info.board_mac_addrl);
								bt_print("\n");
								bt_print("==> Board Serial: 0x");
								bt_print_hexuint(info.board_serial_h);
								bt_print(",0x");
								bt_print_hexuint(info.board_serial_l);
								bt_print("\n");
								bt_print("==> ARM Memory: 0x");
								bt_print_hexuint(info.memory_arm_base);
								bt_print(" {Size:");
								bt_print_int(info.memory_arm_size>>20);
								bt_print("MB}\n");
								bt_print("==> VC Memory: 0x");
								bt_print_hexuint(info.memory_vc_base);
								bt_print(" {Size:");
								bt_print_int(info.memory_vc_size>>20);
								bt_print("MB}\n");
							}
							else if (strncmp(ptmp,"VIDEO",BT_BUFF_SIZE)==0)
							{
								bt_print("[WAIT] ");
								bt_print("Gathering video graphics info... ");
								mailbox_get_video_info(&info);
								bt_print("done.\n");
								bt_print("[INFO] Graphics {0x");
								bt_print_hexuint(info.info_status);
								bt_print("}\n");
								bt_print("==> Physical Dimension: ");
								bt_print_int(info.fb_width);
								bt_print("x");
								bt_print_int(info.fb_height);
								bt_print("\n");
								bt_print("==> Virtual Dimension: ");
								bt_print_int(info.fb_vwidth);
								bt_print("x");
								bt_print_int(info.fb_vheight);
								bt_print("\n");
								bt_print("==> Depth: ");
								bt_print_int(info.fb_depth);
								bt_print(", Pixel Order: ");
								if (info.fb_pixel_order) bt_print("RGB");
								else bt_print("BGR");
								bt_print("\n");
								bt_print("==> Pitch: ");
								bt_print_int(info.fb_pitch);
								bt_print(", Alpha Mode: ");
								switch(info.fb_alpha_mode)
								{
									case 0x00:
										bt_print("Enabled (0-opaque)");
										break;
									case 0x01:
										bt_print("Reversed (0-transparent)");
										break;
									case 0x02:
										bt_print("Disabled (ignored)");
										break;
									default:
										bt_print("Invalid!");
								}
								bt_print("\n");
								bt_print("==> X-Offset: ");
								bt_print_int(info.fb_vx_offset);
								bt_print(", Y-Offset: ");
								bt_print_int(info.fb_vy_offset);
								bt_print("\n");
							}
							else
							{
								bt_print("[ERROR] Invalid info task! {");
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
						bt_print("} (");
						bt_print_int(temp);
						bt_print(")\n");
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
