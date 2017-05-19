/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"
/*----------------------------------------------------------------------------*/
#define MY_LED 47
/*----------------------------------------------------------------------------*/
#define LED_ON gpio_set
#define LED_OFF gpio_clr
/*----------------------------------------------------------------------------*/
#define BLINK_RATE (TIMER_S/2)
/*----------------------------------------------------------------------------*/
void blink(int gpio, int count)
{
	while(count>0)
	{
		LED_ON(gpio);
		timer_wait(BLINK_RATE);
		LED_OFF(gpio);
		timer_wait(BLINK_RATE);
		count--;
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	int check, color = 0xff0000;
	/** initialize gpio */
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	LED_OFF(MY_LED);
	timer_init();
	/* initialize video stuff */
	mailbox_init();
	check = video_init(0x0);
	if (check)
	{
		/* on error, blink led and hang around */
		blink(MY_LED,check);
		while(1);
	}
	/** do the thing... */
	check = 3; /* show rgb */
	while(check>0)
	{
		video_clear(color);
		color >>= 8;
		blink(MY_LED,3);
		check--;
	}
	video_clear(0xaaaaaa); /* show light gray? */
	blink(MY_LED,3);
	/** write something */
	video_put_char(1,1,'H');
	video_put_char(1,2,'E');
	video_put_char(1,3,'L');
	video_put_char(1,4,'L');
	video_put_char(1,5,'O');
	video_put_char(1,6,'!');
	video_put_string(3,1,"Welcome!");
	/** sticking around */
	while(1);
}
/*----------------------------------------------------------------------------*/
