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
#define ENABLE_PIN 4
#define RTC_I2C_ADDR 0x68
/*----------------------------------------------------------------------------*/
void main(void)
{
	fb_t* display;
	int loop, test, data[8], next = 0, flag = 1;
	/** initialize gpio */
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	gpio_config(ENABLE_PIN,GPIO_INPUT);
	gpio_pull(ENABLE_PIN,GPIO_PULL_UP);
	gpio_setevent(ENABLE_PIN,GPIO_EVENT_AEDGR);
	gpio_rstevent(ENABLE_PIN);
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
	/** say something... */
	video_text_string("---------\n");
	video_text_string("I2C Test!\n");
	video_text_string("---------\n");
	/** main loop */
	while(1)
	{
		video_text_cursor(5,0);
		test = i2c_gets(RTC_I2C_ADDR,0x00,data,8);
		for (loop=0;loop<8;loop++)
		{
			video_text_string("Location");
			video_text_integer(loop);
			video_text_string(": 0x");
			video_text_hexbyte((unsigned char)data[loop]);
			video_text_string(" {");
			video_text_integer(test);
			video_text_string("}        \n");
		}
		video_text_string("\nNext: ");
		video_text_integer(next++);
		video_text_string("            \n");
		if (gpio_chkevent(ENABLE_PIN))
		{
			if (flag) data[0] |= 0x80;
			else data[0] &= 0x7F;
			flag = !flag;
			i2c_puts(RTC_I2C_ADDR,0x00,data,1);
			gpio_rstevent(ENABLE_PIN);
		}
	}
}
/*----------------------------------------------------------------------------*/
