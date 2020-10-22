/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uartbb.h"
#include "string.h"
#include "utils.h"
#ifdef USE_OLED
#include "i2c.h"
#include "oled1306.h"
#else
#include "video.h"
#endif
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 128
/*----------------------------------------------------------------------------*/
#define GPS_GREET "MY1BAREPI GPS"
#define NMEA_DELIM ",\r"
/*----------------------------------------------------------------------------*/
typedef struct _gps_data_t
{
	char *plat, *plon, *dutc, *tutc, *pvel, *pang;
	char *pchk, *p_ns, *p_ew, *psum, *buff, *pmag;
	char conv[32];
	float vlat, vlon;
	int wait, skip;
#ifdef USE_OLED
	oled1306_t *oled;
#endif
}
gps_data_t;
/*----------------------------------------------------------------------------*/
void show_gpsdata(gps_data_t* gpsd)
{
	if (gpsd->pchk)
	{
		if (gpsd->pchk[0]=='A')
		{
#ifdef USE_OLED
			oled1306_cursor(gpsd->oled,2,2);
			oled1306_text(gpsd->oled,gpsd->pchk);
			oled1306_cursor(gpsd->oled,3,0);
			oled1306_text(gpsd->oled,"# ");
			oled1306_text(gpsd->oled,gpsd->tutc);
			oled1306_cursor(gpsd->oled,4,0);
			oled1306_text(gpsd->oled,"# ");
			oled1306_text(gpsd->oled,gpsd->plat);
			oled1306_text(gpsd->oled,gpsd->p_ns);
			oled1306_cursor(gpsd->oled,5,0);
			oled1306_text(gpsd->oled,"# ");
			oled1306_text(gpsd->oled,gpsd->plon);
			oled1306_text(gpsd->oled,gpsd->p_ew);
			oled1306_cursor(gpsd->oled,6,0);
			oled1306_text(gpsd->oled,"# ");
			oled1306_text(gpsd->oled,gpsd->dutc);
			oled1306_cursor(gpsd->oled,7,0);
			oled1306_text(gpsd->oled,"# ");
			gpsd->vlat = str2float(gpsd->plat);
			float2str(gpsd->conv,gpsd->vlat,4);
			oled1306_text(gpsd->oled,gpsd->conv);
			//gpsd->vlon = str2float(gpsd->plon);
			//float2str(gpsd->conv,gpsd->vlon,4);
			//oled1306_text(gpsd->oled,gpsd->conv);

			oled1306_update(gpsd->oled);
#else
			video_text_string("FOUND => ");
			video_text_string(gpsd->pchk);
			video_text_string("\n-- Chk: ");
			video_text_string(gpsd->dutc);
			video_text_string(" ");
			video_text_string(gpsd->tutc);
			video_text_string("\n-- Lat: ");
			video_text_string(gpsd->plat);
			video_text_string(gpsd->p_ns);
			video_text_string("\n-- Lng: ");
			video_text_string(gpsd->plon);
			video_text_string(gpsd->p_ew);
			video_text_string("\n\n");
#endif
			gpsd->wait = 0;
		}
		else
		{
			if (!gpsd->skip)
			{
#ifdef USE_OLED
				oled1306_cursor(gpsd->oled,2,2);
				oled1306_text(gpsd->oled,gpsd->pchk);
				oled1306_update(gpsd->oled);
#else
				video_text_string("VOID!\n@@ Buff:{");
				video_text_string(gpsd->buff);
				video_text_string("}\n\n");
#endif
				gpsd->wait = 0;
				gpsd->skip = 10;
			}
			else gpsd->skip--;
		}
	}
	else
	{
#ifdef USE_OLED
		oled1306_cursor(gpsd->oled,2,2);
		oled1306_text(gpsd->oled,"* ");
		oled1306_update(gpsd->oled);
#else
		video_text_string("HUH?!\n@@ Buff:{");
		video_text_string(gpsd->buff);
		video_text_string("}\n\n");
#endif
		gpsd->wait = 0;
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
#ifdef USE_OLED
	oled1306_t oled;
	char prompt[4];
#endif
	char buff[BUFF_SIZE], copy[BUFF_SIZE], *pbuf, *ptmp;
	gps_data_t gpsd;
	int test, temp, size;
	/** initialize stuffs */
	timer_init();
	/** initialize uartbb */
	uartbb_init(26,19); /** gpio26=rx, gpio19=tx */
#ifdef USE_OLED
	/** initialize i2c */
	i2c_init(I2C_SDA1_GPIO,I2C_SCL1_GPIO);
	i2c_set_wait_time(1);
	i2c_set_free_time(3);
	/** initialize oled display */
	oled1306_init(&oled,SSD1306_ADDRESS,OLED_TYPE_128x64,
		find_font_oled(UUID_FONT_OLED_8x8));
	oled1306_clear(&oled);
	oled1306_cursor(&oled,0,0);
	oled1306_text(&oled,GPS_GREET);
	oled1306_update(&oled);
	gpsd.oled = &oled;
	prompt[0] = '>';
	prompt[1] = ' ';
	prompt[2] = 0x0;
#else
	/* hdmi display */
	video_init(VIDEO_RES_VGA);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_cursor(1,0);
	video_text_string(GPS_GREET);
	video_text_cursor(3,0);
#endif
	/** do the thing... */
	gpsd.buff = buff;
	gpsd.skip = 0; gpsd.wait = 0;
	size = 0;
	while(1)
	{
		if (!gpsd.wait)
		{
#ifdef USE_OLED
			oled1306_cursor(&oled,2,0);
			if (prompt[0]=='>') prompt[0] = '|';
			else prompt[0] = '>';
			oled1306_text(&oled,prompt);
			oled1306_update(&oled);
#else
			video_text_string("Waiting... ");
#endif
			gpsd.wait = 1;
		}
		test = uartbb_read();
		if (!size&&test!='$') continue;
		if (!test)
		{
			buff[size] = 0x0;
#ifdef USE_OLED
			oled1306_cursor(&oled,2,2);
			oled1306_text(&oled,"? ");
			oled1306_update(&oled);
#else
			video_text_string("MISSED!\n** Buff:{");
			video_text_string(buff);
			video_text_string(":");
			video_text_integer(size);
			video_text_string("/");
			video_text_integer(BUFF_SIZE);
			video_text_string("}\n\n");
#endif
			size = 0;
			gpsd.wait = 0;
			continue;
		}
		buff[size++] = (unsigned char) test;
		if (size==BUFF_SIZE)
		{
			buff[BUFF_SIZE-1] = 0x0;
#ifdef USE_OLED
			oled1306_cursor(&oled,2,2);
			oled1306_text(&oled,"$ ");
			oled1306_update(&oled);
#else
			video_text_string("OVERFLOW!\n** Buff:{");
			video_text_string(buff);
			video_text_string(":");
			video_text_integer(size);
			video_text_string("/");
			video_text_integer(BUFF_SIZE);
			video_text_string("}\n\n");
#endif
			size = 0;
			gpsd.wait = 0;
			continue;
		}
		if (test!='\n') continue;
		buff[size-1] = 0x0;
		size = 0;
		/** process */
		gpsd.pchk = 0x0;
		if (!strncmp(buff,"$GPRMC",6))
		{ /** alternatives are GGA or GLL */
			strncpy(copy,buff,BUFF_SIZE);
			pbuf = copy; temp = 1;
			strword(&pbuf,NMEA_DELIM);
			while ((ptmp=strword(&pbuf,NMEA_DELIM)))
			{
				switch (temp)
				{
					case 1: gpsd.tutc = ptmp; break;
					case 2: gpsd.pchk = ptmp; break;
					case 3: gpsd.plat = ptmp; break;
					case 4: gpsd.p_ns = ptmp; break;
					case 5: gpsd.plon = ptmp; break;
					case 6: gpsd.p_ew = ptmp; break;
					case 7: gpsd.pvel = ptmp; break;
					case 8: gpsd.pang = ptmp; break;
					case 9: gpsd.dutc = ptmp; break;
				}
				temp++;
			}
		}
		/** ignore other data? */
		if (gpsd.pchk) show_gpsdata(&gpsd);
		size = 0;
	}
}
/*----------------------------------------------------------------------------*/
