/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "i2c.h"
#ifdef USE_OLED
#include "oled1306.h"
#else
#include "video.h"
#include "utils.h"
#endif
/*----------------------------------------------------------------------------*/
#define RTC_I2C_ADDR 0x68
#define ENABLE_PIN 4
/*----------------------------------------------------------------------------*/
#define BOUNCE_DELAY 5000000
/*----------------------------------------------------------------------------*/
int flag, mask;
/*----------------------------------------------------------------------------*/
void pin_handler(void)
{
	if (gpio_chkevent(ENABLE_PIN))
	{
		gpio_setevent(ENABLE_PIN,0);
		gpio_rstevent(ENABLE_PIN);
		if (!mask) flag ^= 0x80;
	}
}
/*----------------------------------------------------------------------------*/
#define RTC_GREET "MY1BAREPI RTC"
/*----------------------------------------------------------------------------*/
void main(void)
{
#ifdef USE_OLED
	oled1306_t oled;
#endif
	unsigned int *psys, *pirq;
	unsigned int prev;
	int loop, next;
	unsigned char data[8];
	/** init gpio */
	gpio_setevent(ENABLE_PIN,0);
	gpio_rstevent(ENABLE_PIN);
	gpio_config(ENABLE_PIN,GPIO_INPUT);
	gpio_pull(ENABLE_PIN,GPIO_PULL_UP);
	/** initialize timer */
	timer_init();
	/** initialize i2c */
	i2c_init(I2C_SDA1_GPIO,I2C_SCL1_GPIO);
	i2c_set_wait_time(1);
	i2c_set_free_time(3);
#ifdef USE_OLED
	/** initialize oled display */
	oled1306_init(&oled,SSD1306_ADDRESS,OLED_TYPE_128x64,
		find_font_oled(UUID_FONT_OLED_8x8));
	oled1306_clear(&oled);
	oled1306_cursor(&oled,0,0);
	oled1306_text(&oled,RTC_GREET);
	oled1306_update(&oled);
#else
	video_init(VIDEO_RES_MAX); /** or, VIDEO_RES_VGA */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_cursor(2,2);
	video_text_string(RTC_GREET);
#endif
	/* prepare stuffs */
	i2c_gets(RTC_I2C_ADDR,0x00,data,1);
	flag = data[0]&0x80; mask = 0; next = 0;
	prev = timer_read();
	psys = (unsigned int*)SYS1FLAG_ADDR;
	if (*psys==BOOTLOADER_ID)
	{
		/* hack for handle_irq in bootloader */
		pirq = (unsigned int*)SYS7FLAG_ADDR;
		pirq = (unsigned int*)(*pirq);
		*pirq = (unsigned int) pin_handler;
	}
	else pirq = 0x0;
	gpio_setevent(ENABLE_PIN,GPIO_EVENT_AEDGR);
	/** main loop */
	while(1)
	{
#ifdef USE_OLED
		oled1306_cursor(&oled,2,0);
#else
		video_text_cursor(4,2);
#endif
		i2c_gets(RTC_I2C_ADDR,0x00,data,8);
		for (loop=0;loop<4;loop++)
		{
#ifdef USE_OLED
			oled1306_text_hexbyte(&oled,data[loop<<1]);
			oled1306_text(&oled,"  ");
			oled1306_text_hexbyte(&oled,data[(loop<<1)+1]);
			oled1306_char(&oled,'\n');
#else
			video_text_hexbyte(data[loop<<1]);
			video_text_string("  ");
			video_text_hexbyte(data[(loop<<1)+1]);
			video_text_string("\n");
#endif
		}
#ifdef USE_OLED
		oled1306_cursor(&oled,7,0);
		oled1306_text(&oled,"Next: ");
		oled1306_text_integer(&oled,next);
		oled1306_text(&oled,"  ");
		oled1306_update(&oled);
#else
		video_text_cursor(9,0);
		video_text_string("Next: ");
		video_text_integer(next);
		video_text_string("  ");
#endif
		next++; if (next>=100) next = 0;
		if (!pirq) pin_handler();
		if ((data[0]&0x80)!=flag)
		{
			if (flag) data[0] |= 0x80;
			else data[0] &= 0x7F;
			i2c_puts(RTC_I2C_ADDR,0x00,data,1);
			mask++;
			prev = timer_read();
		}
		else
		{
			if (mask)
			{
				if (timer_read()-prev>BOUNCE_DELAY)
				{
					mask = 0;
					gpio_setevent(ENABLE_PIN,GPIO_EVENT_AEDGR);
				}
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
