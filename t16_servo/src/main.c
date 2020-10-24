/*----------------------------------------------------------------------------*/
#include "pwm.h"
#include "timer.h"
#include "i2c.h"
#include "oled1306.h"
/*----------------------------------------------------------------------------*/
/** towerpro mg995 (50hz pwm):
 *  - 0.5ms => 0deg
 *  - 1.5ms => 90deg
 *  - 2.5ms => 180deg
 * Operating voltage range: 4.8 V to 7.2 V
 * Stall torque: 9.4kg/cm (4.8v); 11kg/cm (6v)
 * Operating speed: 0.2 s/60º (4.8 V), 0.16 s/60º (6 V)
 * Rotational degree: 180º
 * Dead band width: 5 μs
 * Operating temperature range: 0ºC to +55ºC
 * Current draw at idle: 10mA
 * No load operating current draw: 170mA
 * Current at maximum load: 1200mA
**/
/*----------------------------------------------------------------------------*/
void main(void)
{
	oled1306_t oled;
	unsigned int curr,tmax;
	/** pwm stuffs - 10khz clock @ 0.1ms */
	pwm_init(PWM_CHANNEL_1,1920,0);
	curr = pwm_curr_data(PWM_CHANNEL_1);
	tmax = pwm_curr_full(PWM_CHANNEL_1);
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
	oled1306_text(&oled,"MY1BAREPI PWM   ");
	oled1306_cursor(&oled,2,0);
	oled1306_text(&oled,"CURR: 0x");
	oled1306_text_hexuint(&oled,curr);
	oled1306_cursor(&oled,3,0);
	oled1306_text(&oled,"TMAX: 0x");
	oled1306_text_hexuint(&oled,tmax);
	curr = 15; /* 1.5ms */
	tmax = 200; /* 20ms => 50Hz */
	pwm_prep_data(PWM_CHANNEL_1,curr);
	pwm_prep_full(PWM_CHANNEL_1,tmax);
	oled1306_cursor(&oled,5,0);
	oled1306_text(&oled,"CURR: ");
	oled1306_text_integer(&oled,curr);
	oled1306_cursor(&oled,6,0);
	oled1306_text(&oled,"TMAX: ");
	oled1306_text_integer(&oled,tmax);
	/* start things up */
	pwm_exec(PWM_CHANNEL_1);
	while(1)
	{
		/** do your stuff */
		curr = 5;
		pwm_prep_data(PWM_CHANNEL_1,curr);
		oled1306_cursor(&oled,5,6);
		oled1306_text_integer(&oled,curr);
		oled1306_text(&oled,"        ");
		oled1306_update(&oled);
		timer_wait(3*TIMER_S);
		curr = 15;
		pwm_prep_data(PWM_CHANNEL_1,curr);
		oled1306_cursor(&oled,5,6);
		oled1306_text_integer(&oled,curr);
		oled1306_text(&oled,"        ");
		oled1306_update(&oled);
		timer_wait(3*TIMER_S);
		curr = 25;
		pwm_prep_data(PWM_CHANNEL_1,curr);
		oled1306_cursor(&oled,5,6);
		oled1306_text_integer(&oled,curr);
		oled1306_text(&oled,"        ");
		oled1306_update(&oled);
		timer_wait(3*TIMER_S);
	}
}
/*----------------------------------------------------------------------------*/
