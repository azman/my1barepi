/*----------------------------------------------------------------------------*/
#include "boot.h"
#include "gpio.h"
#include "video.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
#define GPIO_TEST 47
/*----------------------------------------------------------------------------*/
void main(void)
{
	char buff[16];
	int count = 0;
	gpio_config(GPIO_TEST,GPIO_OUTPUT);
	gpio_clr(GPIO_TEST);
	video_init(VIDEO_RES_MAX); /** or, VIDEO_RES_VGA */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_cursor(1,1);
	video_text_string("MY1BAREPI VIDEO LIBRARY");
	video_text_cursor(3,1);
	video_text_string("Count: ");
	int2str(buff,count);
	video_text_string(buff);
	/* test line drawing */
	video_draw_line(100,100,200,100);
	video_draw_line(200,100,100,200);
	video_draw_line(100,200,100,100);
	while (1)
	{
		loopd(0x200000);
		count++;
		int2str(buff,count);
		video_text_cursor(3,8);
		video_text_string(buff);
		/* clear up... just in case */
		video_text_string("                ");
		gpio_toggle(GPIO_TEST);
	}
}
/*----------------------------------------------------------------------------*/
