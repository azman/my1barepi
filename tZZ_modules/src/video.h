/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEOH__
#define __MY1VIDEOH__
/*----------------------------------------------------------------------------*/
#define VIDEO_INIT_RETRIES 3
#define VIDEO_INITIALIZED 0
#define VIDEO_ERROR_RETURN 1
#define VIDEO_ERROR_POINTER 2
/*----------------------------------------------------------------------------*/
/* default video mode - VGA 24-bit RGB */
#define VIDEO_HEIGHT 480
#define VIDEO_WIDTH 640
#define VIDEO_PIXEL_BITS 24
#define VIDEO_PIXEL_SIZE (VIDEO_PIXEL_BITS/8)
/*----------------------------------------------------------------------------*/
typedef struct __fbinfo_t
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
typedef struct __screen_t
{
	int xres, yres;
	int bpp, psize, pskip;
}
screen_t;
/*----------------------------------------------------------------------------*/
typedef struct __cursor_t
{
	int x, y;
	int xmax, ymax;
}
cursor_t;
/*----------------------------------------------------------------------------*/
typedef unsigned int rgb_t;
typedef unsigned char gry_t;
typedef unsigned char pix_t;
/*----------------------------------------------------------------------------*/
int video_init(fbinfo_t* p_fbinfo);
void video_clear(rgb_t color);
void video_set_pixel(int y, int x, rgb_t color);
void video_put_char(int y, int x, char c);
void video_put_string(int y, int x, char* str);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
