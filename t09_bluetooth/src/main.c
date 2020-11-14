/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "bluez.h"
#include "string.h"
#include "utils.h"
#include "mailbox.h"
#include "video.h"
/*----------------------------------------------------------------------------*/
#define FLAG_DEBUG 0x80
/*----------------------------------------------------------------------------*/
void do_print_str(char *msg)
{
	video_text_string(msg);
	bt_print(msg);
}
/*----------------------------------------------------------------------------*/
void do_print_int(int val)
{
	video_text_integer(val);
	bt_print_int(val);
}
/*----------------------------------------------------------------------------*/
void do_print_hex(unsigned int val)
{
	video_text_hexuint(val);
	bt_print_hexuint(val);
}
/*----------------------------------------------------------------------------*/
void show_input(char* pbuf)
{
	char *ptmp, copy[BT_BUFF_SIZE];
	int temp = 0;
	/* use local copy! */
	while (pbuf[temp])
	{
		copy[temp] = pbuf[temp];
		temp++;
	}
	pbuf = copy;
	pbuf[temp] = 0x0;
	/* show the whole buffer */
	do_print_str("[Input] '");
	do_print_str(pbuf);
	do_print_str("' (");
	do_print_int(temp);
	do_print_str(")\n");
	/* show each token detected */
	temp = 0;
	while ((ptmp=strword(&pbuf," \t")))
	{
		temp++;
		do_print_str("[Found] Word#");
		do_print_int(temp);
		do_print_str(": '");
		do_print_str(ptmp);
		do_print_str("'\n");
	}
}
/*----------------------------------------------------------------------------*/
void gpio_commands(char* pbuf)
{
	char* ptmp;
	int test;
	do
	{
		/* get gpio select */
		ptmp = strword(&pbuf," \n\r\t");
		if (!ptmp)
		{
			do_print_str("[ERROR] No gpio selected!\n");
			break;
		}
		test = str2int(ptmp);
		if (test<2||test>27||test==14||test==15)
		{
			do_print_str("[ERROR] Invalid gpio selected! {");
			do_print_str(ptmp);
			do_print_str("}\n");
			break;
		}
		/* get gpio task */
		ptmp = strword(&pbuf," \n\r\t");
		if (!ptmp)
		{
			do_print_str("[ERROR] No task for gpio command!\n");
			break;
		}
		else if (strncmp(ptmp,"CONFIG",BT_BUFF_SIZE)==0)
		{
			ptmp = strword(&pbuf," \n\r\t");
			if (!ptmp)
			{
				do_print_str("[ERROR] No option for gpio config!\n");
				break;
			}
			else if (strncmp(ptmp,"IN",BT_BUFF_SIZE)==0)
			{
				gpio_config(test,GPIO_INPUT);
				do_print_str("[GPIO] GPIO");
				do_print_int(test);
				do_print_str(" configured as input!\n");
			}
			else if (strncmp(ptmp,"OUT",BT_BUFF_SIZE)==0)
			{
				gpio_config(test,GPIO_OUTPUT);
				do_print_str("[GPIO] GPIO");
				do_print_int(test);
				do_print_str(" configured as output!\n");
			}
			else
			{
				do_print_str("[ERROR] Invalid option for ");
				do_print_str("gpio config!\n");
				break;
			}
		}
		else if (strncmp(ptmp,"SET",BT_BUFF_SIZE)==0)
		{
			gpio_set(test);
			do_print_str("[GPIO] GPIO");
			do_print_int(test);
			do_print_str(" set to logic HI!\n");
		}
		else if (strncmp(ptmp,"CLR",BT_BUFF_SIZE)==0)
		{
			gpio_clr(test);
			do_print_str("[GPIO] GPIO");
			do_print_int(test);
			do_print_str(" set to logic LO!\n");
		}
		else if (strncmp(ptmp,"READ",BT_BUFF_SIZE)==0)
		{
			do_print_str("[GPIO] GPIO");
			do_print_int(test);
			do_print_str(" is at logic ");
			if (gpio_read(test)) do_print_str("HI");
			else do_print_str("LO");
			do_print_str("!\n");
		}
		else
		{
			do_print_str("[ERROR] Invalid gpio task! {");
			do_print_str(ptmp);
			do_print_str("}\n");
		}
	}
	while (0);
}
/*----------------------------------------------------------------------------*/
void info_commands(char* pbuf)
{
	tags_info_t info;
	char* ptmp;
	do
	{
		/* get info selection */
		ptmp = strword(&pbuf," \n\r\t");
		if (!ptmp)
		{
			do_print_str("[ERROR] ");
			do_print_str("No selection for info command!\n");
			break;
		}
		else if (strncmp(ptmp,"BOARD",BT_BUFF_SIZE)==0)
		{
			do_print_str("[WAIT] ");
			do_print_str("Gathering board info... ");
			mailbox_get_board_info(&info);
			do_print_str("done.\n");
			do_print_str("[INFO] Hardware {0x");
			do_print_hex(info.info_status);
			do_print_str("}\n");
			do_print_str("==> VideoCore Firmware: 0x");
			do_print_hex(info.vc_revision);
			do_print_str("\n");
			do_print_str("==> Board Model: 0x");
			do_print_hex(info.board_model);
			do_print_str("\n");
			do_print_str("==> Board Revision: 0x");
			do_print_hex(info.board_revision);
			do_print_str("\n");
			do_print_str("==> Board MAC Address: 0x");
			do_print_hex(info.board_mac_addrh);
			do_print_str(",0x");
			do_print_hex(info.board_mac_addrl);
			do_print_str("\n");
			do_print_str("==> Board Serial: 0x");
			do_print_hex(info.board_serial_h);
			do_print_str(",0x");
			do_print_hex(info.board_serial_l);
			do_print_str("\n");
			do_print_str("==> ARM Memory: 0x");
			do_print_hex(info.memory_arm_base);
			do_print_str(" {Size:");
			do_print_int(info.memory_arm_size>>20);
			do_print_str("MB}\n");
			do_print_str("==> VC Memory: 0x");
			do_print_hex(info.memory_vc_base);
			do_print_str(" {Size:");
			do_print_int(info.memory_vc_size>>20);
			do_print_str("MB}\n");
		}
		else if (strncmp(ptmp,"VIDEO",BT_BUFF_SIZE)==0)
		{
			do_print_str("[WAIT] ");
			do_print_str("Gathering video graphics info... ");
			mailbox_get_video_info(&info);
			do_print_str("done.\n");
			do_print_str("[INFO] Graphics {0x");
			do_print_hex(info.info_status);
			do_print_str("}\n");
			do_print_str("==> Physical Dimension: ");
			do_print_int(info.fb_width);
			do_print_str("x");
			do_print_int(info.fb_height);
			do_print_str("\n");
			do_print_str("==> Virtual Dimension: ");
			do_print_int(info.fb_vwidth);
			do_print_str("x");
			do_print_int(info.fb_vheight);
			do_print_str("\n");
			do_print_str("==> Depth: ");
			do_print_int(info.fb_depth);
			do_print_str(", Pixel Order: ");
			if (info.fb_pixel_order) do_print_str("RGB");
			else do_print_str("BGR");
			do_print_str("\n");
			do_print_str("==> Pitch: ");
			do_print_int(info.fb_pitch);
			do_print_str(", Alpha Mode: ");
			switch(info.fb_alpha_mode)
			{
				case 0x00:
					do_print_str("Enabled (0-opaque)");
					break;
				case 0x01:
					do_print_str("Reversed (0-transparent)");
					break;
				case 0x02:
					do_print_str("Disabled (ignored)");
					break;
				default:
					do_print_str("Invalid!");
			}
			do_print_str("\n");
			do_print_str("==> X-Offset: ");
			do_print_int(info.fb_vx_offset);
			do_print_str(", Y-Offset: ");
			do_print_int(info.fb_vy_offset);
			do_print_str("\n");
		}
		else
		{
			do_print_str("[ERROR] Invalid info task! {");
			do_print_str(ptmp);
			do_print_str("}\n");
		}
	}
	while (0);
}
/*----------------------------------------------------------------------------*/
void flag_commands(char* pbuf, int *flag)
{
	char* ptmp;
	do
	{
		/* get info selection */
		ptmp = strword(&pbuf," \n\r\t");
		if (!ptmp)
		{
			do_print_str("[ERROR] ");
			do_print_str("No selection for flag command!\n");
			break;
		}
		else if (strncmp(ptmp,"DEBUG",BT_BUFF_SIZE)==0)
		{
			ptmp = strword(&pbuf," \n\r\t");
			if (!ptmp)
			{
				do_print_str("[FLAG] DEBUG ");
				if (*flag&FLAG_DEBUG) do_print_str("ON!\n");
				else do_print_str("OFF!\n");
			}
			else if (strncmp(ptmp,"ON",BT_BUFF_SIZE)==0)
			{
				*flag |= FLAG_DEBUG;
				do_print_str("[FLAG] DEBUG ON (");
				do_print_hex(*flag&FLAG_DEBUG);
				do_print_str(")\n");
			}
			else if (strncmp(ptmp,"OFF",BT_BUFF_SIZE)==0)
			{
				*flag &= ~FLAG_DEBUG;
				do_print_str("[FLAG] DEBUG OFF (");
				do_print_hex(*flag&FLAG_DEBUG);
				do_print_str(")\n");
			}
			else
			{
				do_print_str("[FLAG] Unknown debug task {");
				do_print_str(ptmp);
				do_print_str("}\n");
			}
		}
		else
		{
			do_print_str("[ERROR] Invalid flag task! {");
			do_print_str(ptmp);
			do_print_str("}\n");
		}
	}
	while (0);
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	btmodule_t btdev;
	int temp, flag;
	char *pbuf, *ptmp, copy[BT_BUFF_SIZE];
	/** initialize basics */
	flag = 0;
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	/** video display */
	video_init(VIDEO_RES_VGA);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_string("\nMY1BAREPI BLUETOOTH TEST PROGRAM\n\n");
	/** initialize bt structure */
	btdev.status = 0;
	btdev.bbsize = 0;
	strncpy(btdev.name,"my1bluez",BT_NAME_BUFF);
	strncpy(btdev.cpin,"4444",BT_CPIN_BUFF);
	btdev.vers[0] = 0x0;
	/** check hc-06 interface */
	while (1)
	{
		video_text_string("-- Initializing HC-06... ");
		bt_init(&btdev);
		if (btdev.status>0) break;
		video_text_string("fail!\n");
		timer_wait(TIMER_S);
	}
	video_text_string("Module ready!\n\n");
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
					btdev.bbsize--;
					btdev.buff[btdev.bbsize] = 0x0;
					if (btdev.buff[btdev.bbsize-1]=='\r')
					{
						btdev.bbsize--;
						btdev.buff[btdev.bbsize] = 0x0;
					}
					strncpy(copy,(char*)btdev.buff,BT_BUFF_SIZE);
					pbuf = copy;
					temp = trimws(pbuf,1);
					str2upper(pbuf);
					if (flag&FLAG_DEBUG)
						show_input(pbuf);
					ptmp = strword(&pbuf," \t");
					if (!ptmp)
					{
						do_print_str("[SIGNAL] I AM ALIVE!\n");
					}
					else if (strncmp(ptmp,"GPIO",BT_BUFF_SIZE)==0)
					{
						gpio_commands(pbuf);
					}
					else if (strncmp(ptmp,"INFO",BT_BUFF_SIZE)==0)
					{
						info_commands(pbuf);
					}
					else if (strncmp(ptmp,"FLAG",BT_BUFF_SIZE)==0)
					{
						flag_commands(pbuf,&flag);
					}
					else
					{
						do_print_str("[ERROR] Unknown command! {");
						do_print_str(ptmp);
						do_print_str("} (");
						do_print_int(temp);
						do_print_str(")\n");
					}
					btdev.bbsize = 0;
				}
			}
			else
			{
				bt_purge();
				/* should be paired by now... send word! */
				do_print_str("[ERROR] Buffer overflow! Input purged!\n");
				btdev.bbsize = 0;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
