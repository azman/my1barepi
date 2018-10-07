/*----------------------------------------------------------------------------*/
#include "am2302.h"
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
int am2302_read(int gpio_sens,unsigned int* ptemp,unsigned int* phumi)
{
	unsigned int loop, read, temp, humi, csum, wait;
	/* start protocol! */
	gpio_clr(gpio_sens);
	timer_wait(1000); /* >= 1ms */
	gpio_set(gpio_sens);
	timer_wait(30); /* 20-40us */
	gpio_config(gpio_sens,GPIO_INPUT);
	/* wait low */
	while(gpio_read(gpio_sens)); /* max 80us */
	/* wait high */
	while(!gpio_read(gpio_sens)); /* max 80us */
	/* wait low */
	while(gpio_read(gpio_sens)); /* prep data */
	/* prepare for calc! */
	read = 0; temp = 0; humi = 0; csum = 0;
	/* get humidity */
	for(loop=0;loop<16;loop++)
	{
		humi <<= 1;
		/* wait high */
		while(!gpio_read(gpio_sens)); /* 26-28us = 0, 70us = 1 */
		wait = timer_read();
		/* wait low */
		while(gpio_read(gpio_sens)); /* prep data */
		/* check logic '1' */
		if(timer_read()-wait>50) humi |= 0x01;
	}
	read = ((humi & 0xFF00) >> 8 ) + (humi & 0xFF);
	/* get temperature */
	for(loop=0;loop<16;loop++)
	{
		temp <<= 1;
		/* wait high */
		while(!gpio_read(gpio_sens)); /* 26-28us = 0, 70us = 1 */
		wait = timer_read();
		/* wait low */
		while(gpio_read(gpio_sens)); /* prep data */
		if(timer_read()-wait>50) temp |= 0x01;
	}
	read = read + ((temp & 0xFF00) >> 8 ) + (temp & 0xFF);
	read &= 0xff;
	/* get checksum */
	for(loop=0;loop<8;loop++)
	{
		csum <<= 1;
		/* wait high */
		while(!gpio_read(gpio_sens)); /* 26-28us = 0, 70us = 1 */
		wait = timer_read();
		/* wait low */
		while(gpio_read(gpio_sens)); /* prep data */
		if(timer_read()-wait>50) csum |= 0x01;
	}
	/* set back to high??? */
	gpio_config(gpio_sens,GPIO_OUTPUT);
	gpio_set(gpio_sens);
	/* assign to provided addresses */
	*phumi = humi; *ptemp = temp;
	/* return checksum results */
	return (read==csum) ? 0 : -1;
}
/*----------------------------------------------------------------------------*/
