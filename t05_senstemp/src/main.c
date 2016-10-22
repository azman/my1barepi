/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "7seg.h"
/*----------------------------------------------------------------------------*/
#define SENS_IN 4
/*----------------------------------------------------------------------------*/
#define USE_COMMON_ANODE
/*----------------------------------------------------------------------------*/
#ifdef USE_COMMON_ANODE
/* common anode */
#define LED_ON 0
#define LED_OFF 1
#else
/* common cathode */
#define LED_ON 1
#define LED_OFF 0
#endif
/*----------------------------------------------------------------------------*/
int segment_pattern[10][SEGMENT_COUNT] =
{
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF }, /* 0 */
	{ LED_OFF,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_OFF,LED_OFF }, /* 1 */
	{ LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON,LED_OFF,LED_ON }, /* 2 */
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_ON }, /* 3 */
	{ LED_OFF,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_ON,LED_ON }, /* 4 */
	{ LED_ON,LED_OFF,LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON }, /* 5 */
	{ LED_ON,LED_OFF,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON }, /* 6 */
	{ LED_ON,LED_ON,LED_ON,LED_OFF,LED_OFF,LED_OFF,LED_OFF }, /* 7 */
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON,LED_ON }, /* 8 */
	{ LED_ON,LED_ON,LED_ON,LED_ON,LED_OFF,LED_ON,LED_ON }  /* 9 */
};
/*----------------------------------------------------------------------------*/
int sensor_read(int gpio_sens,unsigned int* ptemp,unsigned int* phumi)
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
void main(void)
{
	unsigned int temp, humi, init;
	seven_seg_t seg1 = { 0x0, { 5, 6, 7, 8, 9, 10, 11 }, 12 };
	seven_seg_t seg2 = { 0x0, { 20, 21, 22, 23, 24, 25, 26 }, 27 };
	/* init gpio */
	gpio_init();
	gpio_config(SENS_IN,GPIO_OUTPUT);
	gpio_pull(SENS_IN,GPIO_PULL_NONE);
	gpio_set(SENS_IN);
	/* init 7segment interface */
	seven_init(&seg1,segment_pattern[0]);
	seven_init(&seg2,segment_pattern[0]);
	seven_full(&seg1,0,LED_OFF);
	seven_full(&seg2,0,LED_ON);
	/* init timer */
	timer_init();
	/* powerup - wait 1s */
	timer_wait(1000000);
	/** do the thing... */
	while(1)
	{
		/* mark sensor read time */
		init = timer_read();
		/* read sensor value(s) */
		sensor_read(SENS_IN,&temp,&humi);
		/* just display temperature for now */
		humi = humi / 10;
		temp = temp / 10;
		seven_show(&seg1,temp/10);
		seven_show(&seg2,temp%10);
		/* toggle dp - simple indicator */
		gpio_toggle(seg1.gpio_dp);
		gpio_toggle(seg2.gpio_dp);
		/** 2 seconds between read */
		while(timer_read()-init<2000000);
	}
}
/*----------------------------------------------------------------------------*/
