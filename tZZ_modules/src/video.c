/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#include "video.h"
#include "mailbox.h"
#include "barrier.h"
/*----------------------------------------------------------------------------*/
#define VIDEO_FB_CHANNEL MAIL_CH_FBUFF
/*----------------------------------------------------------------------------*/
/* fb_info must be 16-byte aligned! lower 4-bits address is zero! */
fbinfo_t fbinfo __attribute__ ((aligned(16)));
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
			framebuff.cursor.xmax = p_fbinfo->width / font_sys->width;
			framebuff.cursor.ymax = p_fbinfo->height / font_sys->height;
			framebuff.info = p_fbinfo;
			framebuff.font = font_sys;
			framebuff.buff = (pix_t*) (p_fbinfo->pointer);
			framebuff.fgcol = COLOR_WHITE;
			framebuff.bgcol = COLOR_BLACK;
			test = VIDEO_INITIALIZED;
			break;
		}
		init--;
	}
	return test;
}
/*----------------------------------------------------------------------------*/
fbinfo_t* video_get_fbinfo(void)
{
	return framebuff.info;
}
/*----------------------------------------------------------------------------*/
screen_t* video_get_screen(void)
{
	return &framebuff.screen;
}
/*----------------------------------------------------------------------------*/
cursor_t* video_get_cursor(void)
{
	return &framebuff.cursor;
}
/*----------------------------------------------------------------------------*/
font_t* video_get_font(void)
{
	return framebuff.font;
}
/*----------------------------------------------------------------------------*/
void rgb_split(rgb_t color, gry_t *red, gry_t *green, gry_t *blue)
{
	*red = (color&0xff0000)>>16;
	*green = (color&0xff00)>>8;
	*blue = (color&0xff);
}
/*----------------------------------------------------------------------------*/
rgb_t rgb_merge(gry_t red, gry_t green, gry_t blue)
{
	rgb_t merge = blue;
	merge |= green << 8;
	merge |= red << 16;
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
void video_clear(void)
{
	int row, col, idx, dox, doy;
	gry_t r,g,b;
	rgb_split(framebuff.bgcol,&r,&g,&b);
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
rgb_t video_get_pixel(int y, int x)
{
	gry_t r,g,b;
	int index = video_get_index(y,x);
	b = framebuff.buff[index++];
	g = framebuff.buff[index++];
	r = framebuff.buff[index++];
	return rgb_merge(r,g,b);
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
