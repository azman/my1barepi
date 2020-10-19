/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "utils.h"
#include "i2c.h"
#include "oled1306.h"
/*----------------------------------------------------------------------------*/
#define ERROR_LED 47
#define ENABLE_PIN 4
/*----------------------------------------------------------------------------*/
void main(void)
{
	oled1306_t oled;
	int loop;
	/** initialize gpio */
	gpio_config(ERROR_LED,GPIO_OUTPUT);
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
	/* initialize oled display */
	oled1306_init(&oled,SSD1306_ADDRESS,OLED_TYPE_128x64,
		find_font_oled(UUID_FONT_OLED_8x8));
	oled1306_cursor(&oled,0,0);
	oled1306_fill(&oled);
	oled1306_update(&oled);
	timer_wait(3000000);

	oled1306_clear(&oled);
	oled1306_update(&oled);
	timer_wait(3000000);

	for (loop=0;loop<4;loop++)
	{
		oled1306_char(&oled,'A'+loop);
		oled1306_update(&oled);
		timer_wait(1000000);
	}

	for (loop=0;loop<128;loop++)
		oled1306_draw_pixel(&oled,16,loop,1);
	oled1306_update(&oled);
	timer_wait(3000000);


	oled1306_cursor(&oled,4,0);
	oled1306_text(&oled,"Init PGT302");
	oled1306_update(&oled);
	/** main loop */
	while(1)
	{
		//oled1306_cursor(&oled,2,0);
		//oled1306_text(&oled,"Whadda?");
		//oled1306_update(&oled);
	}
}
/*----------------------------------------------------------------------------*/
