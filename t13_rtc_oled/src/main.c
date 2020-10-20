/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "i2c.h"
#include "oled1306.h"
/*----------------------------------------------------------------------------*/
#define RTC_I2C_ADDR 0x68
#define ENABLE_PIN 4
/*----------------------------------------------------------------------------*/
#define VIEW_DELAY 2000000
/*----------------------------------------------------------------------------*/
void main(void)
{
	oled1306_t oled;
	int loop, flag, data[8];
	unsigned char next;
	/** init gpio */
	flag = 0; next = 0;
	gpio_config(ENABLE_PIN,GPIO_INPUT);
	gpio_pull(ENABLE_PIN,GPIO_PULL_UP);
	gpio_setevent(ENABLE_PIN,GPIO_EVENT_AEDGR);
	gpio_rstevent(ENABLE_PIN);
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
	oled1306_clear(&oled);
	oled1306_text(&oled,"MY1BAREPI RTC");
	oled1306_update(&oled);
	/** main loop */
	while(1)
	{
		oled1306_cursor(&oled,2,0);
		i2c_gets(RTC_I2C_ADDR,0x00,data,8);
		for (loop=0;loop<4;loop++)
		{
			oled1306_text_hexbyte(&oled,(unsigned char)data[loop<<1]);
			oled1306_text(&oled,"  ");
			oled1306_text_hexbyte(&oled,(unsigned char)data[(loop<<1)+1]);
			oled1306_char(&oled,'\n');
		}
		oled1306_cursor(&oled,7,0);
		oled1306_text(&oled,"Next: ");
		oled1306_text_integer(&oled,(int)next++);
		oled1306_text(&oled,"   ");
		oled1306_update(&oled);
		if (gpio_chkevent(ENABLE_PIN))
		{
			if (flag) data[0] |= 0x80;
			else data[0] &= 0x7F;
			flag = !flag;
			i2c_puts(RTC_I2C_ADDR,0x00,data,1);
			gpio_rstevent(ENABLE_PIN);
		}
	}
}
/*----------------------------------------------------------------------------*/
