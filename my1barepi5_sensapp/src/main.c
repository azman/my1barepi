/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "uart.h"
#include "timer.h"
#include "utils.h"
/*----------------------------------------------------------------------------*/
#define SENS_IN 4
#define WAIT_1S 1000000
/*----------------------------------------------------------------------------*/
void main(void)
{
	volatile unsigned int test, loop, read, temp, humi, csum, wait;
	unsigned char disp[32];
	float value;
	gpio_init();
	gpio_config(SENS_IN,GPIO_OUTPUT);
	gpio_pull(SENS_IN,GPIO_PULL_NONE);
	gpio_set(SENS_IN);
	uart_init();
	timer_init();
	/** send out the word! */
	uart_print("Temperature & Humidity Monitoring\n");
	/* powerup - wait 1s */
	timer_wait(WAIT_1S);
	/** do the thing... */
	while(1)
	{
		gpio_clr(SENS_IN);
		uart_print("Sending request...\n");
		timer_wait(1000); /* >= 1ms */
		gpio_set(SENS_IN);
		timer_wait(30); /* 20-40us */
		gpio_config(SENS_IN,GPIO_INPUT);
		/* wait low */
		while(gpio_read(SENS_IN)); /* max 80us */
		/* wait high */
		while(!gpio_read(SENS_IN)); /* max 80us */
		/* wait low */
		while(gpio_read(SENS_IN)); /* prep data */
		/* prepare for calc! */
		read = 0; temp = 0; humi = 0; csum = 0;
		/* get humidity */
		for(loop=0;loop<16;loop++)
		{
			humi <<= 1;
			/* wait high */
			while(!gpio_read(SENS_IN)); /* 26-28us = 0, 70us = 1 */
			wait = timer_read();
			/* wait low */
			while(gpio_read(SENS_IN)); /* prep data */
			/* check logic '1' */
			if(timer_read()-wait>50) humi |= 0x01;
		}
		read = ((humi & 0xFF00) >> 8 ) + (humi & 0xFF);
		/* get temperature */
		for(loop=0;loop<16;loop++)
		{
			temp <<= 1;
			/* wait high */
			while(!gpio_read(SENS_IN)); /* 26-28us = 0, 70us = 1 */
			wait = timer_read();
			/* wait low */
			while(gpio_read(SENS_IN)); /* prep data */
			if(timer_read()-wait>50) temp |= 0x01;
		}
		read = read + ((temp & 0xFF00) >> 8 ) + (temp & 0xFF);
		read &= 0xff;
		/* get checksum */
		for(loop=0;loop<8;loop++)
		{
			csum <<= 1;
			/* wait high */
			while(!gpio_read(SENS_IN)); /* 26-28us = 0, 70us = 1 */
			wait = timer_read();
			/* wait low */
			while(gpio_read(SENS_IN)); /* prep data */
			if(timer_read()-wait>50) csum |= 0x01;
		}
		/* set back to high??? */
		gpio_config(SENS_IN,GPIO_OUTPUT);
		gpio_set(SENS_IN);
		/* do checksum */
		if(read!=csum)
			uart_print("Invalid checksum!\n");
		/* print everything for now */
		uart_print("Humidity: ");
		value = humi / 10.0;
		float2str(disp,value);
		uart_print(disp);
		uart_print(" C, Temperature: ");
		value = temp / 10.0;
		float2str(disp,value);
		uart_print(disp);
		uart_print(" %%\n");
	}
}
/*----------------------------------------------------------------------------*/
