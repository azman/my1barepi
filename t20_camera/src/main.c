/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"
#include "utils.h"
#include "font.h"
#include "i2c.h"
/*----------------------------------------------------------------------------*/
#define ERROR_LED 47
#define PIN_HREF 5
#define PIN_VSYN 6
#define PIN_PCLK 13
/*----------------------------------------------------------------------------*/
#define CAM_I2C_ADDR 0x60
/*----------------------------------------------------------------------------*/
void main(void)
{
	fb_t* display;
	int pcnt = 0, hcnt = 0, htmp = 0, vcnt = 0, flag = 0;
	int init = 0, data;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	/** initialize timer */
	timer_init();
	/** initialize i2c */
	i2c_init(I2C_SDA1_GPIO,I2C_SCL1_GPIO);
	/** initialize mailbox */
	mailbox_init();
	/** initialize video */
	display = video_init(VIDEO_RES_VGA);
	/* blink ERROR_LED indefinitely if failed to init */
	if (!display)
	{
		while(1)
		{
			gpio_toggle(ERROR_LED);
			timer_wait(TIMER_S/2);
		}
	}
	/* setup screen */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_string("------------\n");
	video_text_string("Camera Test!\n");
	video_text_string("------------\n\n");
	/* original param */
	video_text_string("COMA: 0x");
	data = i2c_getb(CAM_I2C_ADDR,0x12);
	video_text_hexbyte((unsigned char)data);
	video_text_string(", COMH: 0x");
	data = i2c_getb(CAM_I2C_ADDR,0x28);
	video_text_hexbyte((unsigned char)data);
	video_text_string(", CLKRC: 0x");
	data = i2c_getb(CAM_I2C_ADDR,0x11);
	video_text_hexbyte((unsigned char)data);
	video_text_string("\n");
	/* reset camera */
	i2c_putb(CAM_I2C_ADDR,0x12,0x80);
	/* adjust clock prescaler clk=main/((prc+1)*2) */
	i2c_putb(CAM_I2C_ADDR,0x11,0x0f); /* clk=main/32? */
	/* enable rgb (default 16-bit mode) */
	i2c_putb(CAM_I2C_ADDR,0x12,0x2C);
	/* select 1-line rgb */
	i2c_putb(CAM_I2C_ADDR,0x28,0x81);
	/* adjusted param */
	video_text_string("COMA: 0x");
	data = i2c_getb(CAM_I2C_ADDR,0x12);
	video_text_hexbyte((unsigned char)data);
	video_text_string(", COMH: 0x");
	data = i2c_getb(CAM_I2C_ADDR,0x28);
	video_text_hexbyte((unsigned char)data);
	video_text_string(", CLKRC: 0x");
	data = i2c_getb(CAM_I2C_ADDR,0x11);
	video_text_hexbyte((unsigned char)data);
	video_text_string("\n");
	/* initial data */
	video_text_cursor(10,1);
	video_text_string("Width: ");
	video_text_integer(hcnt);
	video_text_string(", ");
	video_text_string("Height: ");
	video_text_integer(vcnt);
	video_text_string(" (");
	video_text_integer(pcnt);
	video_text_string(":");
	video_text_integer(flag);
	video_text_string(")");
	/** do initialization */
	gpio_config(PIN_HREF,GPIO_INPUT);
	gpio_setevent(PIN_HREF,GPIO_EVENT_AEDGR);
	gpio_config(PIN_VSYN,GPIO_INPUT);
	gpio_setevent(PIN_VSYN,GPIO_EVENT_AEDGR);
	gpio_config(PIN_PCLK,GPIO_INPUT);
	gpio_setevent(PIN_PCLK,GPIO_EVENT_AEDGR);
	/** main loop */
	while(1)
	{
		/** do your stuff */
		if (!init)
		{
			if (gpio_chkevent(PIN_VSYN))
			{
				init = 1;
				hcnt = 0; vcnt = 0; pcnt = 0; flag = 0;
				gpio_rstevent(PIN_PCLK);
				gpio_rstevent(PIN_HREF);
				gpio_rstevent(PIN_VSYN);
			}
			continue;
		}
		if (gpio_chkevent(PIN_PCLK))
		{
			if (flag)
			{
				/** only of href is high */
				htmp++; pcnt++;
			}
			gpio_rstevent(PIN_PCLK);
		}
		if (gpio_chkevent(PIN_HREF))
		{
			flag = !flag;
			if (flag) /* +ve edge */
			{
				/* detect falling edge next */
				gpio_setevent(PIN_HREF,GPIO_EVENT_AEDGF);
				htmp = 0;
			}
			else /* -ve edge */
			{
				gpio_setevent(PIN_HREF,GPIO_EVENT_AEDGR);
				hcnt = htmp;
				vcnt++;
			}
			gpio_rstevent(PIN_HREF);
		}
		if (gpio_chkevent(PIN_VSYN))
		{
			/* show all */
			video_text_cursor(10,1);
			video_text_string("Width: ");
			video_text_integer(hcnt);
			video_text_string(", ");
			video_text_string("Height: ");
			video_text_integer(vcnt);
			video_text_string(" (");
			video_text_integer(pcnt);
			video_text_string(":");
			video_text_integer(htmp);
			video_text_string(":");
			video_text_integer(flag);
			video_text_string(")    ");
			hcnt = 0; vcnt = 0; pcnt = 0;
			gpio_rstevent(PIN_VSYN);
			init = 0;
		}
	}
}
/*----------------------------------------------------------------------------*/
