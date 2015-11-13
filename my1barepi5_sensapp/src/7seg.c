/*----------------------------------------------------------------------------*/
#include "7seg.h"
/*----------------------------------------------------------------------------*/
void seven_init(seven_seg_t *segment,int *pattern,int *gpio,int gpio_dp)
{
	segment->p_pattern = pattern;
	/* should we check for valid gpio number?? */
	segment->gpio_num = gpio;
	segment->gpio_dp = gpio_dp;
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
