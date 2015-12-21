/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
#define MY_SWI 15
/*----------------------------------------------------------------------------*/
#define SEGMENT_A 0
#define SEGMENT_B 1
#define SEGMENT_C 2
#define SEGMENT_D 3
#define SEGMENT_E 4
#define SEGMENT_F 5
#define SEGMENT_G 6
#define SEGMENT_COUNT 7
/*----------------------------------------------------------------------------*/
int segment_pattern[10][SEGMENT_COUNT] =
{
	{ 0,0,0,0,0,0,1 }, /* 0 */
	{ 1,0,0,1,1,1,1 }, /* 1 */
	{ 0,0,1,0,0,1,0 }, /* 2 */
	{ 0,0,0,0,1,1,0 }, /* 3 */
	{ 1,0,0,1,1,0,0 }, /* 4 */
	{ 0,1,0,0,1,0,0 }, /* 5 */
	{ 0,1,0,0,0,0,0 }, /* 6 */
	{ 0,0,0,1,1,1,1 }, /* 7 */
	{ 0,0,0,0,0,0,0 }, /* 8 */
	{ 0,0,0,0,1,0,0 }  /* 9 */
};
/*----------------------------------------------------------------------------*/
int segment_gpio[SEGMENT_COUNT] = { 2,3,4,17,27,22,10 };
/*----------------------------------------------------------------------------*/
void show_number(int number)
{
	volatile unsigned int loop;
	if(number<0||number>9) number = 0;
	for(loop=0;loop<SEGMENT_COUNT;loop++)
	{
		if(segment_pattern[number][loop])
			gpio_set(segment_gpio[loop]);
		else
			gpio_clr(segment_gpio[loop]);
	}
}
/*----------------------------------------------------------------------------*/
void delay_this(unsigned int wait)
{
	unsigned int init = timer_read();
	while((timer_read()-init)<wait);
}
/*----------------------------------------------------------------------------*/
#define DELAY_S 1000000
/*----------------------------------------------------------------------------*/
void main(void)
{
	int count;
	gpio_init();
	gpio_config(segment_gpio[SEGMENT_A],GPIO_OUTPUT);
	gpio_config(segment_gpio[SEGMENT_B],GPIO_OUTPUT);
	gpio_config(segment_gpio[SEGMENT_C],GPIO_OUTPUT);
	gpio_config(segment_gpio[SEGMENT_D],GPIO_OUTPUT);
	gpio_config(segment_gpio[SEGMENT_E],GPIO_OUTPUT);
	gpio_config(segment_gpio[SEGMENT_F],GPIO_OUTPUT);
	gpio_config(segment_gpio[SEGMENT_G],GPIO_OUTPUT);
	gpio_config(MY_SWI,GPIO_INPUT);
	timer_init();
	while(1)
	{
		show_number(0);
		if(gpio_read(MY_SWI)) continue;
		for(count=0;count<10;count++)
		{
			show_number(count);
			delay_this(DELAY_S);
		}
	}
}
/*----------------------------------------------------------------------------*/
