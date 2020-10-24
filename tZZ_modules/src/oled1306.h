/*----------------------------------------------------------------------------*/
#ifndef __MY1OLED1306H__
#define __MY1OLED1306H__
/*----------------------------------------------------------------------------*/
#define OLED_TYPE_128x64 8
#define OLED_TYPE_128x32 4
/*----------------------------------------------------------------------------*/
#define OLED_FLAG_OK 0
#define OLED_FLAG_ERROR -1
/*----------------------------------------------------------------------------*/
#define SSD1306_ADDRESS 0x3C
/*----------------------------------------------------------------------------*/
/* control byte mask - Co bit (*_MORE seems to have no use?) */
#define SSD1306_MASK_MORE	0x80
#define SSD1306_MASK_DATA	0x40
/* control byte options */
#define SSD1306_COMMAND	0x00
#define SSD1306_DATA	(SSD1306_MASK_DATA)
/*----------------------------------------------------------------------------*/
#include "i2c.h"
#include "font_oled.h"
/*----------------------------------------------------------------------------*/
typedef struct _oled1306_t
{
	int addr, type, flag, temp;
	/* buffer is pages of 128 columns (1 page=8 rows) */
	/* - for 128x64 module => 8 pages! */
	/* - for 128x32 module => 4 pages! */
	unsigned char buff[128*8]; /* 1024 */
	int rows, cols;
	font_oled_t *font;
	int xpos, ypos; /* cursor position */
	int xmax, ymax;
}
oled1306_t;
/*----------------------------------------------------------------------------*/
void oled1306_switch(oled1306_t* oled, int on);
void oled1306_init(oled1306_t* oled, int addr, int type, font_oled_t* font);
void oled1306_cursor(oled1306_t* oled, int ypos, int xpos);
void oled1306_clear(oled1306_t* oled);
void oled1306_fill(oled1306_t* oled);
void oled1306_draw_pixel(oled1306_t* oled, int y, int x, int z);
void oled1306_cursor_next(oled1306_t* oled);
void oled1306_cursor_endl(oled1306_t* oled);
void oled1306_char(oled1306_t* oled, char show);
void oled1306_text_hexbyte(oled1306_t* oled, unsigned char byte);
void oled1306_text_hexuint(oled1306_t* oled, unsigned int dwrd);
void oled1306_text(oled1306_t* oled, char* text);
void oled1306_text_integer(oled1306_t* oled, int value);
void oled1306_update(oled1306_t* oled);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
