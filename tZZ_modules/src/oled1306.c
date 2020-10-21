/*----------------------------------------------------------------------------*/
#include "oled1306.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
void oled1306_command(oled1306_t* oled, unsigned char command)
{
	i2c_do_start();
	i2c_do_write_byte(oled->addr<<1);
	i2c_do_write_byte(SSD1306_COMMAND);
	i2c_do_write_byte(command);
	i2c_do_stop();
}
/*----------------------------------------------------------------------------*/
void oled1306_switch(oled1306_t* oled, int on)
{
	if (on) oled1306_command(oled,0xAF); /** display on */
	else oled1306_command(oled,0xAE); /** display off */
}
/*----------------------------------------------------------------------------*/
void oled1306_init(oled1306_t* oled, int addr, int type, font_oled_t* font)
{
	oled->addr = addr & 0x7F;
	switch (type)
	{
		case OLED_TYPE_128x64:
		case OLED_TYPE_128x32: oled->type = type; break;
		default: oled->type = OLED_TYPE_128x64; break;
	}
	oled->flag = OLED_FLAG_OK;
	oled->rows = oled->type * 8;
	oled->cols = 128;
	/** oled->temp = 0; */
	oled1306_command(oled,0xAE); /** display off */
	/* INIT: software initialization flow - from datasheet */
	oled1306_command(oled,0xA8); /** set mux ratio? default is 63? 0x3f? */
	oled1306_command(oled,0x3F); /* this is default??? */
	oled1306_command(oled,0xD3); /** set display offset */
	oled1306_command(oled,0x00); /* no offset? */
	oled1306_command(oled,0x40 | 0x00); /** set start line (value mask:0x3f) */
	oled1306_command(oled,0xA0 | 0x01); /** set segment remap (value:0|1) */
	oled1306_command(oled,0xC0 | 0x08); /** set com scan dir (vmask:0x00) */
	oled1306_command(oled,0xDA); /** set com pins hw config */
	oled1306_command(oled,0x12); /* sample use 0x12? datasheet use 0x02 */
	oled1306_command(oled,0x81); /** set contrast control */
	oled1306_command(oled,0xCF); /* sample use 0xCF? datasheet use 0x7F */
	oled1306_command(oled,0xA4); /** disable 'entire display on' - o/p ram */
	oled1306_command(oled,0xA6); /** set normal display */
	oled1306_command(oled,0xD5); /** set clock divider ratio & osc freq */
	oled1306_command(oled,0x80); /* freq:0x80(0x00) | cdiv:0x00(0x01) */
	oled1306_command(oled,0x8D); /** set charge pump */
	oled1306_command(oled,0x14); /* internal vcc */
	/* ENDS: software initialization flow - from datasheet */
	oled1306_command(oled,0x20); /** set memory addressing mode*/
	oled1306_command(oled,0x00); /* horizontal: autoinc col, autoinc page */
	oled1306_command(oled,0xD9); /** set precharge period */
	oled1306_command(oled,0xF1);
	oled1306_command(oled,0xDB); /** set vcom deselect */
	oled1306_command(oled,0x40);
	oled1306_command(oled,0x2E); /** STOP SCROLLING? */
	oled1306_command(oled,0xAF); /** display on */
	oled->font = font;
	oled->xpos = 0;
	oled->ypos = 0;
	oled->xmax = oled->cols/font->cols;
	oled->ymax = oled->type; /** 64@32/font->rows; */
}
/*----------------------------------------------------------------------------*/
void oled1306_cursor(oled1306_t* oled, int ypos, int xpos)
{
	if (ypos<oled->ymax&&xpos<oled->xmax)
	{
		oled->ypos = ypos;
		oled->xpos = xpos;
	}
}
/*----------------------------------------------------------------------------*/
void oled1306_clear(oled1306_t* oled)
{
	int loop;
	for (loop=0; loop<1024; loop++)
		oled->buff[loop] = 0;
}
/*----------------------------------------------------------------------------*/
void oled1306_fill(oled1306_t* oled)
{
	int loop;
	for (loop=0; loop<1024; loop++)
		oled->buff[loop] = 0xff;
}
/*----------------------------------------------------------------------------*/
void oled1306_draw_pixel(oled1306_t* oled, int y, int x, int z)
{
	int mask = 0x01;
	int page = y / 8;
	int bits = y % 8;
	if (y<oled->rows&&x<oled->cols)
	{
		mask <<= bits;
		if (z) oled->buff[page*oled->cols+x] |= mask;
		else oled->buff[page*oled->cols+x] &= ~mask;
	}
}
/*----------------------------------------------------------------------------*/
void oled1306_cursor_next(oled1306_t* oled)
{
	oled->xpos++;
	if (oled->xpos>=oled->xmax)
	{
		oled->xpos = 0;
		if (oled->ypos<oled->ymax)
			oled->ypos++;
		/* else, should scroll the screen! :p */
	}
}
/*----------------------------------------------------------------------------*/
void oled1306_cursor_endl(oled1306_t* oled)
{
	oled->xpos = 0;
	if (oled->ypos<oled->ymax)
		oled->ypos++;
	/* else, should scroll the screen! :p */
}
/*----------------------------------------------------------------------------*/
void oled1306_char(oled1306_t* oled, char show)
{
	int loop;
	unsigned char *pchr, *pdst;
	if (show=='\n')
	{
		oled1306_cursor_endl(oled);
		return;
	}
	pchr = (unsigned char*) &oled->font->data[(int)show<<3];
	pdst = &oled->buff[oled->ypos*128+oled->xpos*oled->font->cols];
	for (loop=0; loop<8; loop++)
		pdst[loop] = pchr[loop];
	oled1306_cursor_next(oled);
}
/*----------------------------------------------------------------------------*/
void oled1306_text_hexbyte(oled1306_t* oled, unsigned char byte)
{
	oled1306_char(oled,byte2hex(byte,1,1));
	oled1306_char(oled,byte2hex(byte,0,1));
}
/*----------------------------------------------------------------------------*/
void oled1306_text_hexuint(oled1306_t* oled, unsigned int dwrd)
{
	int loop, pass = 32;
	unsigned int temp;
	for (loop=0;loop<4;loop++)
	{
		pass -= 8;
		temp = dwrd;
		temp >>= pass;
		temp &= 0xff;
		oled1306_text_hexbyte(oled,(unsigned char)temp);
	}
}
/*----------------------------------------------------------------------------*/
void oled1306_text(oled1306_t* oled, char* text)
{
	while (*text) oled1306_char(oled,*text++);
}
/*----------------------------------------------------------------------------*/
void oled1306_text_integer(oled1306_t* oled, int value)
{
	char temp[16];
	int2str(temp,value);
	oled1306_text(oled,temp);
}
/*----------------------------------------------------------------------------*/
void oled1306_update(oled1306_t* oled)
{
	int loop;
	i2c_do_start();
	i2c_do_write_byte(oled->addr<<1);
	i2c_do_write_byte(SSD1306_COMMAND);
	i2c_do_write_byte(0x22); /* set page */
	i2c_do_write_byte(0); /* page 0 */
	i2c_do_write_byte(7); /* page 7 */
	i2c_do_write_byte(0x21); /* set cols */
	i2c_do_write_byte(0); /* column 0 */
	i2c_do_write_byte(127); /* column 127 */
	i2c_do_stop();
	i2c_do_start();
	i2c_do_write_byte(oled->addr<<1);
	i2c_do_write_byte(SSD1306_DATA);
	for (loop=0; loop<1024; loop++)
	{
		/* send from buffer! */
		i2c_do_write_byte(oled->buff[loop]);
	}
	i2c_do_stop();
}
/*----------------------------------------------------------------------------*/
