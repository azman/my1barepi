/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "barrier.h"
/*----------------------------------------------------------------------------*/
/** FRAMEBUFFER MODULE BEGIN! */
/*----------------------------------------------------------------------------*/
typedef struct __fbinfo
{
	unsigned int width, height;
	unsigned int vwidth, vheight; /* virtual? */
	unsigned int pitch; /* byte count in a row */
	unsigned int depth; /* bits per pixel */
	unsigned int xoffset, yoffset;
	unsigned int pointer, size;
}
fbinfo_t;
/*----------------------------------------------------------------------------*/
#define VIDEO_FB_CHANNEL MAIL_CH_FBUFF
/*----------------------------------------------------------------------------*/
#define VIDEO_INIT_RETRIES 3
#define VIDEO_INITIALIZED 0
#define VIDEO_ERROR_RETURN 1
#define VIDEO_ERROR_POINTER 2
/*----------------------------------------------------------------------------*/
#define VC_MMU_MAP 0x40000000
/*----------------------------------------------------------------------------*/
int video_init(fbinfo_t *p_fbinfo)
{
	unsigned int init = VIDEO_INIT_RETRIES;
	unsigned int test, addr = ((unsigned int)p_fbinfo)|VC_MMU_MAP;
	while(init>0)
	{
		memory_barrier();
		mailbox_write(VIDEO_FB_CHANNEL,addr);
		memory_barrier();
		test = mailbox_read(VIDEO_FB_CHANNEL);
		memory_barrier();
		if (test) test = VIDEO_ERROR_RETURN;
		else if (p_fbinfo->pointer==0x0) test = VIDEO_ERROR_POINTER;
		else { test = VIDEO_INITIALIZED; break; }
		init--;
	}
	return test;
}
/*----------------------------------------------------------------------------*/
/** FRAMEBUFFFER MODULE END */
/*----------------------------------------------------------------------------*/
#define MY_LED 47
/*----------------------------------------------------------------------------*/
void blink(int gpio, int count)
{
	while(count>0)
	{
		gpio_set(gpio);
		timer_wait(TIMER_S/2);
		gpio_clr(gpio);
		timer_wait(TIMER_S/2);
		count--;
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int *fb, color = 0x00ff0000;
	fbinfo_t fb_info __attribute__((aligned(16)));
	tags_info_t info;
	int loopy,loopx,index,check;
	/** debug! */
	enable_fpu();
	/** initialize gpio */
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	gpio_clr(MY_LED);
	timer_init();
	/** initialize mailbox */
	mailbox_init();
	mailbox_get_video_info(&info);
	/** initialize fbinfo */
	fb_info.height = info.fb_height;
	fb_info.width = info.fb_width;
	fb_info.vheight = info.fb_height;
	fb_info.vwidth = info.fb_width;
	fb_info.pitch = 0;
	fb_info.depth = 32;
	fb_info.xoffset = 0;
	fb_info.yoffset = 0;
	fb_info.pointer = 0;
	fb_info.size = 0;
	loopx = video_init(&fb_info);
	if (loopx)
	{
		/* on error, blink led and hang around */
		blink(MY_LED,loopx);
		while(1);
	}
	/** do the thing... */
	fb = (unsigned int*) (fb_info.pointer);
	check = fb_info.pitch/sizeof(unsigned int);
	while(1)
	{
		index = 0;
		for (loopy=0;loopy<fb_info.height;loopy++)
		{
			for (loopx=0;loopx<fb_info.width||loopx<check;loopx++)
			{
				fb[index++] = color;
			}
		}
		color >>= 8;
		if (!color) color = 0x00ff0000;
		blink(MY_LED,3);
	}
}
/*----------------------------------------------------------------------------*/
