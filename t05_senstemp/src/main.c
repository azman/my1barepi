/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "textlcd.h"
#include "am2302.h"
/*----------------------------------------------------------------------------*/
#define SENS_IN 4
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int temp, humi, init;
	/* init gpio */
	gpio_config(SENS_IN,GPIO_OUTPUT);
	gpio_pull(SENS_IN,GPIO_PULL_NONE);
	gpio_set(SENS_IN);
	/* init timer */
	timer_init();
	lcd_init();
	lcd_goto_line1();
	lcd_print("TEMP/HUMI SENSOR");
	lcd_goto_line2();
	lcd_print("                ");
	/* powerup - wait 1s */
	timer_wait(1000000);
	/** do the thing... */
	while(1)
	{
		/* mark sensor read time */
		init = timer_read();
		/* read sensor value(s) */
		am2302_read(SENS_IN,&temp,&humi);
		/* value is in one-tenth */
		temp /= 10;
		/* show on text lcd */
		lcd_goto_line2();
		lcd_print("T:");
		lcd_send_data((char)(temp/10)+0x30);
		lcd_send_data((char)(temp%10)+0x30);
		lcd_send_data(223);
		lcd_send_data('C');
		/** 2 seconds between read */
		while(timer_read()-init<2000000);
	}
}
/*----------------------------------------------------------------------------*/
