/*----------------------------------------------------------------------------*/
#include "mailbox.h"
#include "gpio.h"
#include "timer.h"
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
extern void memory_barrier(void);
/*----------------------------------------------------------------------------*/
#define VIDEO_INITIALIZED 0
#define VIDEO_ERROR_RETURN 2
#define VIDEO_ERROR_POINTER 4
/*----------------------------------------------------------------------------*/
int video_init(unsigned int fbinfo_addr)
{
	unsigned int test;
	memory_barrier();
	mailbox_write(VIDEO_FB_CHANNEL,fbinfo_addr);
	memory_barrier();
	test = mailbox_read(VIDEO_FB_CHANNEL);
	memory_barrier();
	if (test)
		return VIDEO_ERROR_RETURN;
	return VIDEO_INITIALIZED;
}
/*----------------------------------------------------------------------------*/
/** FRAMEBUFFFER MODULE END */
/*----------------------------------------------------------------------------*/
#define MY_LED 47
/*----------------------------------------------------------------------------*/
#define LED_ON gpio_set
#define LED_OFF gpio_clr
/*----------------------------------------------------------------------------*/
#define WAIT_DELAY (TIMER_S/2)
/*----------------------------------------------------------------------------*/
void main(void)
{
	volatile unsigned char *fb, dummy,chk_r=0xFF,chk_g=0x00,chk_b=0x00;
	volatile fbinfo_t *fb_info = (fbinfo_t*) (1<<22); /* 0x00400000 */
	volatile int loopx, loopy, chk_x, chk_y, psize;
	volatile int loop;
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
	do
	{
		loopx = video_init(((unsigned int)fb_info)|VC_MMU_MAP);
		if (fb_info->pointer==0x0) loopx = VIDEO_ERROR_POINTER;
		if (loopx)
		{
			for (loop=0;loop<loopx;loop++)
			{
				LED_ON(MY_LED);
				timer_wait(WAIT_DELAY);
				LED_OFF(MY_LED);
				timer_wait(WAIT_DELAY);
			}
			continue;
		}
	}
	while (0);
	/** do the thing... */
	psize = fb_info->depth/8; /* pixel size in bytes */
	fb = (volatile unsigned char*) (fb_info->pointer);
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
		for (loop=0;loop<3;loop++)
		{
			LED_ON(MY_LED);
			timer_wait(WAIT_DELAY);
			LED_OFF(MY_LED);
			timer_wait(WAIT_DELAY);
		}
	}
}
/*----------------------------------------------------------------------------*/
