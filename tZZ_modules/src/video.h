/*----------------------------------------------------------------------------*/
#ifndef __MY1VIDEOH__
#define __MY1VIDEOH__
/*----------------------------------------------------------------------------*/
#include "font.h"
/*----------------------------------------------------------------------------*/
#define VIDEO_INIT_RETRIES 3
#define VIDEO_INITIALIZED 0
#define VIDEO_ERROR_RETURN 1
#define VIDEO_ERROR_POINTER 2
/*----------------------------------------------------------------------------*/
/* default video mode - VGA 32-bit RGB */
#define VIDEO_HEIGHT 480
#define VIDEO_WIDTH 640
#define VIDEO_PIXEL_BITS 32
/*----------------------------------------------------------------------------*/
/* constants for basic colors - http://www.w3.org/TR/CSS21/syndata.html */
#define COLOR_WHITE			0xffffff /* ww3name */
#define COLOR_SILVER		0xc0c0c0 /* ww3name */
#define COLOR_LIGHTGRAY		0xc0c0c0
#define COLOR_LIGHTGREY		0xc0c0c0
#define COLOR_GRAY			0x808080 /* ww3name */
#define COLOR_DARKGRAY		0x808080
#define COLOR_DARKGREY		0x808080
#define COLOR_BLACK			0x000000 /* ww3name */
#define COLOR_BLUE			0x0000ff /* ww3name */
#define COLOR_NAVY			0x000080 /* ww3name */
#define COLOR_AQUA			0x00ffff /* ww3name */
#define COLOR_CYAN			0x00ffff
#define COLOR_TEAL			0x008080 /* ww3name */
#define COLOR_FUCHSIA		0xff00ff /* ww3name */
#define COLOR_MAGENTA		0xff00ff
#define COLOR_PURPLE		0x800080 /* ww3name */
#define COLOR_RED			0xff0000 /* ww3name */
#define COLOR_MAROON		0x800000 /* ww3name */
#define COLOR_YELLOW		0xffff00 /* ww3name */
#define COLOR_OLIVE			0x808000 /* ww3name */
#define COLOR_LIME			0x00ff00 /* ww3name */
#define COLOR_GREEN			0x008000 /* ww3name */
/*----------------------------------------------------------------------------*/
typedef struct __screen_t
{
	int xres, yres, xout, yout, xoff, yoff;
	int depth, pskip, fsize;
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
typedef struct __ppoint_t
{
	int x, y;
}
ppoint_t;
/*----------------------------------------------------------------------------*/
typedef unsigned int rgb_t;
typedef unsigned char gry_t;
typedef unsigned char pix_t;
/*----------------------------------------------------------------------------*/
typedef struct __fb_t
{
	screen_t screen;
	cursor_t cursor;
	font_t *font;
	rgb_t *buff;
	rgb_t fgcol, bgcol;
}
fb_t;
/*----------------------------------------------------------------------------*/
#define VIDEO_RES_VGA 0
#define VIDEO_RES_MAX 1
/*----------------------------------------------------------------------------*/
fb_t* video_init(int resolution);
/* basic pixel operation */
void video_clear(void);
void video_set_pixel(int y, int x, rgb_t color);
rgb_t video_get_pixel(int y, int x);
/* color settings */
void video_set_color(rgb_t color);
rgb_t video_get_color(void);
void video_set_bgcolor(rgb_t bgcolor);
rgb_t video_get_bgcolor(void);
/* text mode functions */
void video_text_cursor(int y, int x);
void video_text_newline(void);
void video_text_char(char c);
void video_text_string(char* str);
/* graphics functions */
void video_draw_line(int x1, int y1, int x2, int y2);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
