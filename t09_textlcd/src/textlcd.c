/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "textlcd.h"
/*----------------------------------------------------------------------------*/
void lcd_busy_wait(void) /** wait while lcd is busy! */
{
	/** command, read */
	gpio_clr(LCD_DNC_GPIO);
	gpio_set(LCD_RNW_GPIO);
	gpio_config(LCD_BUSY_GPIO,GPIO_INPUT); /** need to read busy status */
	do { /** PWmin(LCD_ENB) = 230n (from HD44780U datasheet) */
		gpio_set(LCD_ENB_GPIO);
		/* is this enough??? maybe insert 1us wait? */
		gpio_clr(LCD_ENB_GPIO);
	} while(gpio_read(LCD_BUSY_GPIO));
	gpio_config(LCD_BUSY_GPIO,GPIO_OUTPUT); /** restore as output! */
}
/*----------------------------------------------------------------------------*/
void lcd_send_init(lcdbyte command)
{
	/** requires specific time delay!? */
	gpio_put_data(command);
	/** command, write */
	gpio_clr(LCD_DNC_GPIO);
	gpio_clr(LCD_RNW_GPIO);
	gpio_set(LCD_ENB_GPIO);
	gpio_clr(LCD_ENB_GPIO);
}
/*----------------------------------------------------------------------------*/
void lcd_send_command(lcdbyte command)
{
	lcd_busy_wait();
	gpio_put_data(command);
	/** command, write */
	gpio_clr(LCD_DNC_GPIO);
	gpio_clr(LCD_RNW_GPIO);
	gpio_set(LCD_ENB_GPIO);
	gpio_clr(LCD_ENB_GPIO);
}
/*----------------------------------------------------------------------------*/
void lcd_send_data(lcdbyte asciidat)
{
	lcd_busy_wait();
	gpio_put_data(asciidat);
	/** data, write */
	gpio_set(LCD_DNC_GPIO);
	gpio_clr(LCD_RNW_GPIO);
	gpio_set(LCD_ENB_GPIO);
	gpio_clr(LCD_ENB_GPIO);
}
/*----------------------------------------------------------------------------*/
void lcd_init(void)
{
	gpio_clr(LCD_ENB_GPIO);
	/** wait >40ms Vcc=2.7V @ >15ms Vcc=4.5V (from HD44780U datasheet) */
	timer_wait(100*TIMER_MS);
	lcd_send_init(LCD_CMD_INITSEQ);
	/** wait >4.1ms (from HD44780U datasheet) */
	timer_wait(5*TIMER_MS);
	lcd_send_init(LCD_CMD_INITSEQ);
	/** wait >100ums (from HD44780U datasheet) */
	timer_wait(1*TIMER_MS);
	loop_delay(LCD_COUNT_001MS,LCD_CHECK_001MS);
	lcd_send_init(LCD_CMD_INITSEQ);
	/** reset sequence */
	timer_wait(100*TIMER_US);
	lcd_send_init(LCD_CMD_CONF_B8L2D5x7);
	timer_wait(100*TIMER_US);
	lcd_send_init(LCD_CMD_CONF_DISP0);
	timer_wait(100*TIMER_US);
	lcd_send_init(LCD_CMD_CLEAR);
	timer_wait(100*TIMER_US);
	lcd_send_init(LCD_CMD_CONF_CIS0); /** entry mode */
	/** busy flag somehow is still NOT ready! */
	timer_wait(5*TIMER_MS);
	/** busy flag now SHOULD REALLY be usable! */
	lcd_send_command(LCD_CMD_CONF_D1C0B0);
}
/*----------------------------------------------------------------------------*/
void lcd_print(char* message)
{
	while(*message)
	{
		lcd_send_data(*message);
		message++;
	}
}
/*----------------------------------------------------------------------------*/
