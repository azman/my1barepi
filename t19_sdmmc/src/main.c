/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "i2c.h"
#include "oled1306.h"
#include "sdmmc.h"
/*----------------------------------------------------------------------------*/
void main(void)
{
	oled1306_t oled;
	int test;
	unsigned char buffer[512];
	/** oled: timer & i2c required */
	timer_init();
	/** initialize i2c */
	i2c_init(I2C_SDA1_GPIO,I2C_SCL1_GPIO);
	i2c_set_wait_time(1);
	i2c_set_free_time(3);
	/** initialize oled display */
	oled1306_init(&oled,SSD1306_ADDRESS,OLED_TYPE_128x64,
		find_font_oled(UUID_FONT_OLED_8x8));
	oled1306_cursor(&oled,0,0);
	oled1306_clear(&oled);
	oled1306_text(&oled,"MY1BAREPI EMMC  ");
	oled1306_update(&oled);
	do
	{
		/* init card */
		test = sdmmc_init();
		oled1306_cursor(&oled,2,0);
		oled1306_text(&oled,"Init: ");
		if (test)
		{
			oled1306_text_hexuint(&oled,test);
			oled1306_cursor(&oled,3,0);
			oled1306_text(&oled,"Flag: ");
			oled1306_text_hexuint(&oled,get_keep()->flag);
			oled1306_cursor(&oled,4,0);
			oled1306_text(&oled,"Step: ");
			oled1306_text_integer(&oled,get_keep()->step);
			oled1306_cursor(&oled,5,0);
			oled1306_text(&oled,"CMD0: ");
			oled1306_text_hexuint(&oled,get_keep()->cmd0);
			oled1306_cursor(&oled,6,0);
			oled1306_text(&oled,"ARG1: ");
			oled1306_text_hexuint(&oled,get_keep()->arg1);
			oled1306_update(&oled);
			break;
		}
		oled1306_text_hexuint(&oled,get_keep()->flag);
		oled1306_update(&oled);
		/* read 1st block */
		test = sdmmc_readblock(0,buffer);
		if (test==128)
		{
			oled1306_cursor(&oled,4,0);
			oled1306_text(&oled,"Read: ");
			oled1306_text_integer(&oled,test);
			oled1306_cursor(&oled,5,0);
			oled1306_text(&oled,"Byte: 0x");
			oled1306_text_hexbyte(&oled,buffer[510]);
			oled1306_text(&oled," 0x");
			oled1306_text_hexbyte(&oled,buffer[511]);
			oled1306_update(&oled);
		}
		else
		{
			oled1306_cursor(&oled,4,0);
			oled1306_text(&oled,"Stat: ");
			oled1306_text_hexuint(&oled,get_keep()->stat);
			oled1306_update(&oled);
		}
	}
	while (0);

	while (1)
	{
		/** do your stuff */
	}
}
/*----------------------------------------------------------------------------*/
