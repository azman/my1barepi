/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "video.h"
#include "utils.h"
#include "am2302.h"
/*----------------------------------------------------------------------------*/
#define GPIO_TEST 47
#define SENS_IN 4
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int temp, humi, init;
	char buff[16];
	gpio_config(GPIO_TEST,GPIO_OUTPUT);
	gpio_clr(GPIO_TEST);
	gpio_config(SENS_IN,GPIO_OUTPUT);
	gpio_pull(SENS_IN,GPIO_PULL_NONE);
	gpio_set(SENS_IN);
	timer_init();
	init = timer_read(); /* mark bootup time */
	video_init(VIDEO_RES_VGA);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_cursor(2,2);
	video_text_string("Welcome! ");
	/** 1 second boot-up time for sensor */
	while(timer_read()-init<1000000);
	/* main loop */
	while (1)
	{
		/* mark sensor read time */
		init = timer_read();
		/* read sensor value(s) */
		am2302_read(SENS_IN,&temp,&humi);
		int2str(buff,(int)temp/10);
		video_text_cursor(4,2);
		video_text_string("Temperature: ");
		video_text_string(buff);
		video_text_char('.');
		int2str(buff,(int)temp%10);
		video_text_string(buff);
		int2str(buff,(int)humi/10);
		video_text_char('C');
		video_text_cursor(6,2);
		video_text_string("Humidity: ");
		video_text_string(buff);
		video_text_char('.');
		int2str(buff,(int)humi%10);
		video_text_string(buff);
		video_text_char('%');
		gpio_toggle(GPIO_TEST);
		/** 2 seconds between read */
		while(timer_read()-init<2000000);
	}
}
/*----------------------------------------------------------------------------*/
