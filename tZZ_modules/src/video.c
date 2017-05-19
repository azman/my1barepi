/*----------------------------------------------------------------------------*/
#include "video.h"
#include "mailbox.h"
#include "barrier.h"
#include "font.h"
/*----------------------------------------------------------------------------*/
/** L2 cache disabled => 0x40000000 if enabled */
#define VC_MMU_MAP 0xC0000000
/*----------------------------------------------------------------------------*/
#define VIDEO_FB_CHANNEL MAIL_CH_FBUFF
/*----------------------------------------------------------------------------*/
/* fb_info must be 16-byte aligned! lower 4-bits address is zero! */
fbinfo_t fbinfo __attribute__ ((aligned(16)));
/*----------------------------------------------------------------------------*/
typedef struct __fb_t
{
	screen_t screen;
	cursor_t cursor;
	fbinfo_t *info;
	font_t *font;
	pix_t *buff;
}
fb_t;
/*----------------------------------------------------------------------------*/
fb_t framebuff;
/*----------------------------------------------------------------------------*/
extern font_t font_reg;
extern font_t font_big;
font_t* font_sys = &font_big;
/*----------------------------------------------------------------------------*/
int video_init(fbinfo_t *p_fbinfo)
{
	unsigned int init = VIDEO_INIT_RETRIES, test, addr;
	/** initialize default fbinfo if necessary */
	if (!p_fbinfo)
	{
		fbinfo.height = VIDEO_HEIGHT;
		fbinfo.width = VIDEO_WIDTH;
		fbinfo.vheight = VIDEO_HEIGHT;
		fbinfo.vwidth = VIDEO_WIDTH;
		fbinfo.pitch = 0;
		fbinfo.depth = VIDEO_PIXEL_BITS;
		fbinfo.xoffset = 0;
		fbinfo.yoffset = 0;
		fbinfo.pointer = 0;
		fbinfo.size = 0;
		p_fbinfo = &fbinfo;
	}
	addr = ((unsigned int)p_fbinfo)|VC_MMU_MAP;
	while(init>0)
	{
		memory_barrier();
		mailbox_write(VIDEO_FB_CHANNEL,addr);
		memory_barrier();
		test = mailbox_read(VIDEO_FB_CHANNEL);
		memory_barrier();
		if (test) test = VIDEO_ERROR_RETURN;
		else if (p_fbinfo->pointer==0x0) test = VIDEO_ERROR_POINTER;
		else
		{
			/* fill in framebuffer info */
			framebuff.screen.xres = p_fbinfo->width;
			framebuff.screen.yres = p_fbinfo->height;
			framebuff.screen.bpp = p_fbinfo->depth;
			framebuff.screen.psize = p_fbinfo->depth/8;
			framebuff.screen.pskip = p_fbinfo->pitch;
			framebuff.cursor.x = 0;
			framebuff.cursor.y = 0;
			framebuff.cursor.xmax = (p_fbinfo->width / font_sys->width);
			framebuff.cursor.ymax = (p_fbinfo->height / font_sys->height);
			framebuff.info = p_fbinfo;
			framebuff.font = font_sys;
			framebuff.buff = (pix_t*) (p_fbinfo->pointer);
			test = VIDEO_INITIALIZED;
			break;
		}
		init--;
	}
	return test;
}
/*----------------------------------------------------------------------------*/
void rgb_split(rgb_t color, gry_t *red, gry_t *green, gry_t *blue)
{
	*red = (color&0xff0000)>>16;
	*green = (color&0xff00)>>8;
	*blue = (color&0xff);
}
/*----------------------------------------------------------------------------*/
rgb_t rgb_merge(rgb_t color, gry_t red, gry_t green, gry_t blue)
{
	rgb_t merge = blue;
	merge |= red << 8;
	merge |= blue << 16;
	return merge;
}
/*----------------------------------------------------------------------------*/
int video_get_index(int y, int x)
{
	int index = y*framebuff.screen.pskip+framebuff.info->yoffset;
	index += x*framebuff.screen.psize+framebuff.info->xoffset;
	return index;
}
/*----------------------------------------------------------------------------*/
void video_clear(rgb_t color)
{
	int row, col, idx, dox, doy;
	gry_t r,g,b;
	rgb_split(color,&r,&g,&b);
	doy = framebuff.info->yoffset;
	for (row=0;row<framebuff.screen.yres;row++)
	{
		dox = framebuff.info->xoffset;
		for (col=0;col<framebuff.screen.xres;col++)
		{
			idx = doy + dox;
			framebuff.buff[idx++] = b;
			framebuff.buff[idx++] = g;
			framebuff.buff[idx++] = r;
			dox += framebuff.screen.psize;
		}
		doy += framebuff.screen.pskip;
	}
}
/*----------------------------------------------------------------------------*/
void video_set_pixel(int y, int x, rgb_t color)
{
	gry_t r,g,b;
	int index = video_get_index(y,x);
	rgb_split(color,&r,&g,&b);
	framebuff.buff[index++] = b;
	framebuff.buff[index++] = g;
	framebuff.buff[index++] = r;
}
/*----------------------------------------------------------------------------*/
void video_put_char(int y, int x, char c)
{
	int xchar, ychar, xpart, ypart, index, pmask;
	unsigned char *pdata = (unsigned char*) framebuff.font->data, check;
	index = (int)c*framebuff.font->height;
	ypart = y * framebuff.font->height;
	for(ychar=0; ychar<framebuff.font->height; ychar++)
	{
		xpart = x * framebuff.font->width;
		pmask = 1 << (framebuff.font->width-1);
		check = pdata[index+ychar];
		for(xchar=0; xchar<framebuff.font->width; xchar++)
		{
			if(check&pmask)
				video_set_pixel(ypart,xpart,0xffffff);
			else
				video_set_pixel(ypart,xpart,0x0000ff);
			xpart++;
			pmask >>= 1;
		}
		ypart++;
	}
}
/*----------------------------------------------------------------------------*/
void video_put_string(int y, int x, char* str)
{
	while(*str)
	{
		video_put_char(y,x++,*str);
		str++;
	}
}
/*----------------------------------------------------------------------------*/
