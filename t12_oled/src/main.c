/*----------------------------------------------------------------------------*/
#include "timer.h"
#include "i2c.h"
#include "oled1306.h"
/*----------------------------------------------------------------------------*/
#define VIEW_DELAY 2000000
/*----------------------------------------------------------------------------*/
void main(void)
{
	oled1306_t oled;
	int loop;
	unsigned int *psys;
	/** initialize timer */
	timer_init();
	/** initialize i2c */
	i2c_init(I2C_SDA1_GPIO,I2C_SCL1_GPIO);
	i2c_set_wait_time(1);
	i2c_set_free_time(3);
	/** initialize oled display */
	oled1306_init(&oled,SSD1306_ADDRESS,OLED_TYPE_128x64,
		find_font_oled(UUID_FONT_OLED_8x8));
	oled1306_cursor(&oled,0,0);
	oled1306_fill(&oled);
	oled1306_update(&oled);
	timer_wait(VIEW_DELAY);
	oled1306_clear(&oled);
	oled1306_update(&oled);
	timer_wait(VIEW_DELAY);
	for (loop=0;loop<4;loop++)
	{
		oled1306_char(&oled,'A'+loop);
		oled1306_update(&oled);
		timer_wait(VIEW_DELAY/4);
	}
	for (loop=0;loop<128;loop++)
		oled1306_draw_pixel(&oled,16,loop,1);
	oled1306_update(&oled);
	timer_wait(VIEW_DELAY);
	oled1306_cursor(&oled,4,0);
	oled1306_text(&oled,"Init PGT302");
	oled1306_cursor(&oled,5,0);
	psys = (unsigned int*)0x40;
	oled1306_text_hexuint(&oled,*psys);
	oled1306_cursor(&oled,6,0);
	psys++;
	oled1306_text_hexuint(&oled,*psys);
	oled1306_update(&oled);
	/** hangin' around */
	while (1) {}
}
/*----------------------------------------------------------------------------*/
