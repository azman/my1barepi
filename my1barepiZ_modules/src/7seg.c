/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "7seg.h"
/*----------------------------------------------------------------------------*/
void seven_init(seven_seg_t *segment,int *pattern)
{
	volatile unsigned int loop;
	segment->p_pattern = pattern;
	/* gpio pins should be assigned/validated externally */
	for(loop=0;loop<SEGMENT_COUNT;loop++)
		gpio_config(segment->gpio_num[loop],GPIO_OUTPUT);
	gpio_config(segment->gpio_dp,GPIO_OUTPUT);
}
/*----------------------------------------------------------------------------*/
void seven_show(seven_seg_t *segment,int number)
{
	volatile unsigned int loop;
	for(loop=0;loop<SEGMENT_COUNT;loop++)
	{
		if(segment->p_pattern[number*SEGMENT_COUNT+loop])
			gpio_set(segment->gpio_num[loop]);
		else
			gpio_clr(segment->gpio_num[loop]);
	}
}
/*----------------------------------------------------------------------------*/
void seven_decp(seven_seg_t *segment, int dp)
{
	if(segment->gpio_dp!=SEGMENT_DP_NC)
	{
		if(dp) gpio_set(segment->gpio_dp);
		else gpio_clr(segment->gpio_dp);
	}
}
/*----------------------------------------------------------------------------*/
void seven_full(seven_seg_t *segment,int number,int dp)
{
	seven_show(segment,number);
	seven_decp(segment,dp);
}
/*----------------------------------------------------------------------------*/
