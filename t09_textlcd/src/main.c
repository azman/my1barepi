/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "textlcd.h"
/*----------------------------------------------------------------------------*/
#define GPIO_ACT_LED 47
/*----------------------------------------------------------------------------*/
void main(void)
{
	char fillo = '.'; int check = 0;
	/** do initialization */
	gpio_config(GPIO_ACT_LED,GPIO_OUTPUT);
	gpio_set(GPIO_ACT_LED);
	timer_init();
	lcd_init();
	lcd_send_command(LCD_POS_LINE1);
	lcd_print("MY1BAREPI LCDCHK");
	lcd_send_command(LCD_POS_LINE2);
	/** main loop */
	while(1)
	{
		/** do your stuff */
		lcd_send_data(fillo); check++;
		if(check==LCD_MAX_CHAR)
		{
			check = 0;
			lcd_send_command(LCD_POS_LINE2);
			if(fillo=='.') fillo = '+';
			else fillo = '.';
		}
		timer_wait(TIMER_S);
		gpio_toggle(GPIO_ACT_LED);
	}
}
/*----------------------------------------------------------------------------*/
