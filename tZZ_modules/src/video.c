/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#include "video.h"
#include "mailbox.h"
#include "barrier.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
#define VIDEO_FB_CHANNEL MAIL_CH_FBUFF
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
fb_t framebuff;
/*----------------------------------------------------------------------------*/
extern font_t font_reg;
extern font_t font_big;
font_t* font_sys = &font_big;
/*----------------------------------------------------------------------------*/
fb_t* video_init(int resolution)
{
	unsigned int init = VIDEO_INIT_RETRIES, test, addr;
	fbinfo_t fb_info __attribute__ ((aligned(16)));
	tags_info_t info;
	/** get screen information */
	mailbox_init();
	mailbox_get_video_info(&info);
	/** initialize fbinfo */
	fb_info.height = info.fb_height;
	fb_info.width = info.fb_width;
	fb_info.vheight = resolution ? info.fb_height : VIDEO_HEIGHT;
	fb_info.vwidth = resolution ? info.fb_width : VIDEO_WIDTH;
	fb_info.pitch = 0;
	fb_info.depth = VIDEO_PIXEL_BITS;
	fb_info.xoffset = 0;
	fb_info.yoffset = 0;
	fb_info.pointer = 0;
	fb_info.size = 0;
	addr = ((unsigned int)&fb_info)|VC_MMU_MAP;
	while(init>0)
	{
		memory_barrier();
		mailbox_write(VIDEO_FB_CHANNEL,addr);
		memory_barrier();
		test = mailbox_read(VIDEO_FB_CHANNEL);
		memory_barrier();
		if (test) test = VIDEO_ERROR_RETURN;
		else if (fb_info.pointer==0x0) test = VIDEO_ERROR_POINTER;
		else
		{
			/* fill in framebuffer info */
			framebuff.screen.xres = fb_info.vwidth;
			framebuff.screen.yres = fb_info.vheight;
			framebuff.screen.xout = fb_info.width;
			framebuff.screen.yout = fb_info.height;
			framebuff.screen.depth = fb_info.depth;
			framebuff.screen.pskip = fb_info.pitch;
			framebuff.screen.xoff = fb_info.xoffset;
			framebuff.screen.yoff = fb_info.yoffset;
			framebuff.screen.fsize = fb_info.vwidth * fb_info.vheight;
			framebuff.cursor.x = 0;
			framebuff.cursor.y = 0;
			framebuff.cursor.xmax = fb_info.vwidth / font_sys->width;
			framebuff.cursor.ymax = fb_info.vheight / font_sys->height;
			framebuff.font = font_sys;
			framebuff.buff = (rgb_t*) (fb_info.pointer);
			framebuff.fgcol = COLOR_WHITE;
			framebuff.bgcol = COLOR_BLACK;
			test = VIDEO_INITIALIZED;
			break;
		}
		init--;
	}
	return test==VIDEO_INITIALIZED?&framebuff:0x0;
}
/*----------------------------------------------------------------------------*/
void video_clear(void)
{
	int loop;
	for (loop=0;loop<framebuff.screen.fsize;loop++)
		framebuff.buff[loop] = framebuff.bgcol;
}
/*----------------------------------------------------------------------------*/
void video_set_pixel(int y, int x, rgb_t color)
{
	framebuff.buff[y*framebuff.screen.xres+x] = color;
}
/*----------------------------------------------------------------------------*/
rgb_t video_get_pixel(int y, int x)
{
	return framebuff.buff[y*framebuff.screen.xres+x];
}
/*----------------------------------------------------------------------------*/
void video_set_color(rgb_t color)
{
	framebuff.fgcol = color;
}
/*----------------------------------------------------------------------------*/
rgb_t video_get_color(void)
{
	return framebuff.fgcol;
}
/*----------------------------------------------------------------------------*/
void video_set_bgcolor(rgb_t bgcolor)
{
	framebuff.bgcol = bgcolor;
}
/*----------------------------------------------------------------------------*/
rgb_t video_get_bgcolor(void)
{
	return framebuff.bgcol;
}
/*----------------------------------------------------------------------------*/
void video_text_cursor(int y, int x)
{
	while (y>=framebuff.cursor.ymax) y -= framebuff.cursor.ymax;
	framebuff.cursor.y = y;
	while (x>=framebuff.cursor.xmax) x -= framebuff.cursor.xmax;
	framebuff.cursor.x = x;
}
/*----------------------------------------------------------------------------*/
void video_text_newline(void)
{
	rgb_t ptemp;
	int ytemp, xtemp, ypart;
	framebuff.cursor.x = 0;
	framebuff.cursor.y++;
	if (framebuff.cursor.y>=framebuff.cursor.ymax)
	{
		/* assume auto-scroll */
		framebuff.cursor.y = framebuff.cursor.ymax - 1;
		ypart = framebuff.cursor.y * framebuff.font->height;
		/* browse pixels */
		for (ytemp=0;ytemp<framebuff.screen.yres;ytemp++)
		{
			for (xtemp=0;xtemp<framebuff.screen.xres;xtemp++)
			{
				if (ytemp<ypart)
				{
					ptemp = video_get_pixel(ytemp+framebuff.font->height,xtemp);
					video_set_pixel(ytemp,xtemp,ptemp);
				}
				else
				{
					video_set_pixel(ytemp,xtemp,framebuff.bgcol);
				}
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
void video_text_char(char c)
{
	int xchar, ychar, xpart, ypart, index, pmask;
	unsigned char *pdata = (unsigned char*) framebuff.font->data, check;
	if (c=='\n')
	{
		video_text_newline();
		return;
	}
	index = (int)c*framebuff.font->height;
	ypart = framebuff.cursor.y * framebuff.font->height;
	for(ychar=0; ychar<framebuff.font->height; ychar++)
	{
		xpart = framebuff.cursor.x * framebuff.font->width;
		pmask = 1 << (framebuff.font->width-1);
		check = pdata[index+ychar];
		for(xchar=0; xchar<framebuff.font->width; xchar++)
		{
			if(check&pmask)
				video_set_pixel(ypart,xpart,framebuff.fgcol);
			else
				video_set_pixel(ypart,xpart,framebuff.bgcol);
			xpart++;
			pmask >>= 1;
		}
		ypart++;
	}
	/* update cursor */
	framebuff.cursor.x++;
	if (framebuff.cursor.x>=framebuff.cursor.xmax)
	{
		video_text_newline();
	}
}
/*----------------------------------------------------------------------------*/
void video_text_string(char* str)
{
	while(*str)
	{
		video_text_char(*str);
		str++;
	}
}
/*----------------------------------------------------------------------------*/
void video_text_integer(int value)
{
	char temp[16];
	int2str(temp,value);
	video_text_string(temp);
}
/*----------------------------------------------------------------------------*/
void video_text_hexbyte(unsigned char byte)
{
	video_text_char(byte2hex(byte,1,1));
	video_text_char(byte2hex(byte,0,1));
}
/*----------------------------------------------------------------------------*/
void video_text_hexuint(unsigned int dwrd)
{
	int loop, pass = 32;
	unsigned int temp;
	for (loop=0;loop<4;loop++)
	{
		pass -= 8;
		temp = dwrd;
		temp >>= pass;
		temp &= 0xff;
		video_text_hexbyte((unsigned char)temp);
	}
}
/*----------------------------------------------------------------------------*/
void video_draw_line(int x1, int y1, int x2, int y2)
{
	int dx, dy, x, y, s1, s2, e, temp, swap, i;

	dy = y2 - y1;
	if (dy<0) { dy = -dy; s2 = -1; }
	else s2 = 1;
	dx = x2 - x1;
	if (dx<0) { dx = -dx; s1 = -1; }
	else s1 = 1;

	x = x1;
	y = y1;
	if (dy > dx)
	{
		temp=dx;
		dx=dy;
		dy=temp;
		swap=1;
	}
	else swap=0;

	e = 2 * dy - dx;
	for (i=0; i<=dx; i++)
	{
		video_set_pixel(y,x,framebuff.fgcol);
		while (e>=0)
		{
			if (swap==1) x += s1;
			else y += s2;
			e -= 2*dx;
		}
		if (swap==1) y += s2;
		else x += s1;
		e += 2*dy;
	}
}
/*----------------------------------------------------------------------------*/
