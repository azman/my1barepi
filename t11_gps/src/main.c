/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "string.h"
#include "utils.h"
#include "video.h"
#include "fpu.h"
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 128
/*----------------------------------------------------------------------------*/
typedef struct _gps_data_t
{
	char *plat, *plon, *putc, *pchk, *p_ns, *p_ew, *psum;
	float vlat, vlon;
}
gps_data_t;
/*----------------------------------------------------------------------------*/
void main(void)
{
	char buff[BUFF_SIZE], copy[BUFF_SIZE], *pbuf, *ptmp;
	gps_data_t gpsd;
	int test, temp, skip = 0, wait = 0, size = 0;
	/** initialize stuffs */
	//enable_fpu();
	timer_init();
	/** initialize uart */
	uart_init(UART_BAUD_DEFAULT);
	/* display */
	video_init(VIDEO_RES_VGA);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_string("\nMY1BAREPI GPS TEST PROGRAM\n");
	/** do the thing... */
	while(1)
	{
		if (!wait)
		{
			gpsd.vlat = 1.0;
			video_text_string("\nWaiting... ");
			float2str(copy,gpsd.vlat,2);
			video_text_string(copy);
			video_text_string(" => ");
			wait = 1;
		}
		if (uart_incoming())
		{
			test = uart_read();
			if (size<BUFF_SIZE-1)
			{
				buff[size++] = (unsigned char) test;
				if (size>1&&test=='\n')
				{
					buff[size-1] = 0x0;
					if (buff[size-2]=='\r')
						buff[size-2] = 0x0;
					size = 0;
					/** process */
					if (!strncmp(buff,"$GPGLL",6))
					{
						gpsd.pchk = 0x0;
						strncpy(copy,buff,BUFF_SIZE);
						pbuf = copy; temp = 1;
						strword(&pbuf,",\r\n"); /* remove GPGLL */
						while ((ptmp=strword(&pbuf,",\r\n")))
						{
							switch (temp)
							{
								case 1: gpsd.plat = ptmp; break;
								case 2: gpsd.p_ns = ptmp; break;
								case 3: gpsd.plon = ptmp; break;
								case 4: gpsd.p_ew = ptmp; break;
								case 5: gpsd.putc = ptmp; break;
								case 6: gpsd.pchk = ptmp; break;
								case 7: gpsd.psum = ptmp; break;
							}
							temp++;
						}
						if (gpsd.pchk)
						{
							if (gpsd.pchk[0]=='A')
							{
								video_text_string("FOUND!");
								video_text_string("\n-- Time: ");
								video_text_string(gpsd.putc);
								video_text_string("\n-- Lat: ");
								video_text_string(gpsd.plat);
								video_text_string(" (");
								video_text_string(gpsd.p_ns);
								video_text_string(")");
								video_text_string("\n-- Lng: ");
								video_text_string(gpsd.plon);
								video_text_string(" (");
								video_text_string(gpsd.p_ew);
								video_text_string(")");
								wait = 0;
							}
							else
							{
								if (!skip)
								{
									video_text_string("VOID! => {");
									video_text_string(buff);
									video_text_string("}");
									wait = 0;
									skip = 10;
								}
								else skip--;
							}
						}
						else
						{
							video_text_string("HUH?! => {");
							video_text_string(buff);
							video_text_string("}");
							wait = 0;
						}
					}
				}
			}
			else
			{
				buff[BUFF_SIZE-1] = 0x0;
				size = 0;
				video_text_string("OVERFLOW! => {");
				video_text_string(buff);
				video_text_string("}");
				wait = 0;
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
