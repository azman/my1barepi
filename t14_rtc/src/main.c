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
/*----------------------------------------------------------------------------*/
void main(void)
{
	fb_t* display;
	int loop, test, data[8], next = 0;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	/** initialize timer */
	timer_init();
	/** initialize i2c */
	i2c_init_bb(I2C_SDA1_GPIO,I2C_SCL1_GPIO);
	/** initialize mailbox */
	mailbox_init();
	/** initialize video */
	display = video_init(VIDEO_RES_VGA);
	/* blink ERROR_LED indefintely if failed to init */
	if (!display)
	{
		while(1) { gpio_toggle(ERROR_LED); timer_wait(TIMER_S/2); }
	}
	/* setup screen */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/** do initialization */
	data[0] = 0x50; /* ssec=50 */
	data[1] = 0x48; /* mmin=48 */
	data[2] = 0x57; /* hour=17 */
	i2c_puts(0x68,0x00,data,3);
	video_text_string("---------\n");
	video_text_string("I2C Test!\n");
	video_text_string("---------\n");
	/** main loop */
	while(1)
	{
		video_text_cursor(5,0);
		test = i2c_gets(0x68,0x00,data,8);
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
		video_text_string("\nTest: ");
		video_text_integer(i2c_getb(0x68,0x00));
		video_text_string("\n");
	}
}
/*----------------------------------------------------------------------------*/
