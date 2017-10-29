/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "barrier.h"
/*----------------------------------------------------------------------------*/
/** FRAMEBUFFFER MODULE BEGIN! */
/*----------------------------------------------------------------------------*/
typedef struct __fbinfo
{
	unsigned int width, height;
	unsigned int vwidth, vheight; /* virtual? */
	unsigned int pitch; /* byte counts between rows */
	unsigned int depth; /* bits per pixel (24-bits default?) */
	unsigned int xoffset, yoffset;
	unsigned int pointer, size;
}
fbinfo_t;
/*----------------------------------------------------------------------------*/
/** L2 cache disabled => 0x40000000 if enabled */
#define VC_MMU_MAP 0xC0000000
/*----------------------------------------------------------------------------*/
#define VIDEO_HEIGHT 480
#define VIDEO_WIDTH 640
#define VIDEO_PIXEL_BITS 24
#define VIDEO_PIXEL_SIZE (VIDEO_PIXEL_BITS/8)
/*----------------------------------------------------------------------------*/
#define VIDEO_FB_CHANNEL MAIL_CH_FBUFF
/*----------------------------------------------------------------------------*/
#define VIDEO_INIT_RETRIES 3
#define VIDEO_INITIALIZED 0
#define VIDEO_ERROR_RETURN 1
#define VIDEO_ERROR_POINTER 2
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
	unsigned char *fb, dummy,chk_r=0xFF,chk_g=0x00,chk_b=0x00;
	fbinfo_t *fb_info = (fbinfo_t*) (1<<22); /* 0x00400000 */
	int loopx, loopy, chk_x, chk_y, psize;
	/** initialize gpio */
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	LED_OFF(MY_LED);
	timer_init();
	/** initialize fbinfo */
	fb_info->height = VIDEO_HEIGHT;
	fb_info->width = VIDEO_WIDTH;
	fb_info->vheight = VIDEO_HEIGHT;
	fb_info->vwidth = VIDEO_WIDTH;
	fb_info->pitch = 0;
	fb_info->depth = VIDEO_PIXEL_BITS;
	fb_info->xoffset = 0;
	fb_info->yoffset = 0;
	fb_info->pointer = 0;
	fb_info->size = 0;
	/* initialize video stuff */
	mailbox_init();
	loopx = video_init(fb_info);
	if (loopx)
	{
		/* on error, blink led and hang around */
		blink(MY_LED,loopx);
		while(1);
	}
	/** do the thing... */
	psize = fb_info->depth/8; /* pixel size in bytes */
	fb = (unsigned char*) (fb_info->pointer);
	while(1)
	{
		chk_y = fb_info->yoffset;
		for (loopy=0;loopy<fb_info->height;loopy++)
		{
			/** chk_y = loopy*fb_info->pitch + fb_info->yoffset; */
			chk_x = fb_info->xoffset;
			for (loopx=0;loopx<fb_info->width;loopx++)
			{
				/** chk_x = loopx*psize + fb_info->xoffset; */
				fb[chk_y+chk_x+0] = chk_b;
				fb[chk_y+chk_x+1] = chk_g;
				fb[chk_y+chk_x+2] = chk_r;
				chk_x += psize;
			}
			chk_y += fb_info->pitch;
		}
		dummy = chk_b; chk_b = chk_g; chk_g = chk_r; chk_r = dummy;
		blink(MY_LED,3);
	}
}
/*----------------------------------------------------------------------------*/
