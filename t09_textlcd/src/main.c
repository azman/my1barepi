/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "textlcd.h"
/*----------------------------------------------------------------------------*/
void main(void)
{
	char fillo = '.'; int check = 0;
	/** do initialization */
	gpio_init();
	gpio_init_data();
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
	}
}
/*----------------------------------------------------------------------------*/
