/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
/** FRAMEBUFFFER MODULE BEGIN! */
/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define MAILBOX_BASE (PMAP_BASE|MAILBOX_OFFSET)
/*----------------------------------------------------------------------------*/
/** L2 cache disabled => 0x40000000 if enabled */
#define VC_MMU_MAP 0xC0000000
#define MAIL0_BASE    0x0
#define MAIL0_READ    0x0
#define MAIL0_POLL    0x4
#define MAIL0_SEND_ID 0x5
#define MAIL0_STATUS  0x6
#define MAIL0_CONFIG  0x7
#define MAIL0_WRITE   0x8
/* MAIL0_WRITE IS ACTUALLY MAIL1_BASE? */
#define MAIL1_BASE    0x8
#define MAIL1_READ    0x8
#define MAIL1_STATUS  0xE
/*----------------------------------------------------------------------------*/
#define MAIL_STATUS_FULL  0x80000000
#define MAIL_STATUS_EMPTY 0x40000000
#define MAIL_CHANNEL_MASK 0x0000000F
#define MAIL_CH_POWER 0x00000000
#define MAIL_CH_FBUFF 0x00000001
#define MAIL_CH_VUART 0x00000002
#define MAIL_CH_VCHIQ 0x00000003
#define MAIL_CH_LEDS  0x00000004
#define MAIL_CH_BUTTS 0x00000005
#define MAIL_CH_TOUCH 0x00000006
#define MAIL_CH_NOUSE 0x00000007
#define MAIL_CH_TAGAV 0x00000008
#define MAIL_CH_TAGVA 0x00000009
/*----------------------------------------------------------------------------*/
volatile unsigned int *mailbox;
/*----------------------------------------------------------------------------*/
void mailbox_init(void)
{
	mailbox = (unsigned int*) MAILBOX_BASE;
}
/*----------------------------------------------------------------------------*/
unsigned int mailbox_read(unsigned int channel)
{
	unsigned int value;
	while (1)
	{
		/* wait if mailbox is empty */
		while (mailbox[MAIL0_STATUS]&MAIL_STATUS_EMPTY);
		/* get value@channel */
		value = mailbox[MAIL0_BASE];
		/* check if the expected channel */
		if ((value&MAIL_CHANNEL_MASK)==channel) break;
	}
	return (value&~MAIL_CHANNEL_MASK);
}
/*----------------------------------------------------------------------------*/
void mailbox_write(unsigned int channel,unsigned int value)
{
	/* merge value/channel data */
	value &= ~MAIL_CHANNEL_MASK;
	value |= (channel&MAIL_CHANNEL_MASK);
	/* wait if mailbox is full */
	while (mailbox[MAIL0_STATUS]&MAIL_STATUS_FULL); /* not MAIL1_STAT? */
	/* send it! */
	mailbox[MAIL0_WRITE] = value;
}
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
fbinfo;
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
	volatile fbinfo *fb_info = (fbinfo*) (1<<22);
	volatile int loopx, loopy;
	volatile int loop;
	/** initialize gpio */
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	LED_OFF(MY_LED);
	timer_init();
	/** initialize fbinfo */
	fb_info->height = 0;
	fb_info->width = 0;
	fb_info->vheight = VIDEO_WIDTH;
	fb_info->vwidth = VIDEO_HEIGHT;
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
	/** this works, but don't we need (fb_info->pointer&~VC_MMU_MAP)??? */
	fb = (volatile unsigned char*) (fb_info->pointer);
	/** do the thing... */
	while(1)
	{
		for (loopy=0;loopy<VIDEO_HEIGHT;loopy++)
		{
			for (loopx=0;loopx<VIDEO_WIDTH;loopx++)
			{
				fb[loopy*fb_info->pitch+loopx*VIDEO_PIXEL_SIZE+0] = chk_r;
				fb[loopy*fb_info->pitch+loopx*VIDEO_PIXEL_SIZE+1] = chk_g;
				fb[loopy*fb_info->pitch+loopx*VIDEO_PIXEL_SIZE+2] = chk_b;
			}
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
