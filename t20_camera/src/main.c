/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"
#include "utils.h"
#include "font.h"
/*----------------------------------------------------------------------------*/
#define ERROR_LED 47
#define PIN_HREF 5
#define PIN_VSYN 6
#define PIN_PCLK 13
/*----------------------------------------------------------------------------*/
void blink_error(int count)
{
	while(count>0)
	{
		gpio_toggle(ERROR_LED); timer_wait(TIMER_S/2);
		gpio_toggle(ERROR_LED); timer_wait(TIMER_S/2);
		count--;
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	fb_t* display;
	char temp[16];
	int pcnt = 0, hcnt = 0, htmp = 0, vcnt = 0, flag = 0;
	int init = 0;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	/** initialize timer */
	timer_init();
	/** initialize mailbox */
	mailbox_init();
	/** initialize video wait till ok? */
	do
	{
		blink_error(2);
		timer_wait(TIMER_S);
		display = video_init();
	}
	while (!display);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	video_text_string("------------\n");
	video_text_string("Camera Test!\n");
	video_text_string("------------\n\n");
	video_text_cursor(5,1);
	video_text_string("Width: ");
	int2str(temp,hcnt);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("Height: ");
	int2str(temp,vcnt);
	video_text_string(temp);
	video_text_string(" (");
	int2str(temp,pcnt);
	video_text_string(temp);
	video_text_string(":");
	int2str(temp,flag);
	video_text_string(temp);
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
			video_text_cursor(5,1);
			video_text_string("Width: ");
			int2str(temp,hcnt);
			video_text_string(temp);
			video_text_string(", ");
			video_text_string("Height: ");
			int2str(temp,vcnt);
			video_text_string(temp);
			video_text_string(" (");
			int2str(temp,pcnt);
			video_text_string(temp);
			video_text_string(":");
			int2str(temp,htmp);
			video_text_string(temp);
			video_text_string(":");
			int2str(temp,flag);
			video_text_string(temp);
			video_text_string(")    ");
			hcnt = 0; vcnt = 0; pcnt = 0;
			gpio_rstevent(PIN_VSYN);
			init = 0;
		}
	}
}
/*----------------------------------------------------------------------------*/
