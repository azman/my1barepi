/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "7seg.h"
/*----------------------------------------------------------------------------*/
void seven_init(seven_seg_t *segment, int *pattern)
{
	volatile unsigned int loop;
	/* assign pattern if not already assigned */
	if (!segment->pattern) segment->pattern = pattern;
	/* gpio pins should be assigned/validated externally */
	for (loop=0;loop<SEGMENT_COUNT;loop++)
		gpio_config(segment->gpio_num[loop],GPIO_OUTPUT);
	gpio_config(segment->gpio_dp,GPIO_OUTPUT);
	/* check segment type - default is SEGMENT_COMMON_ANODE */
	switch (segment->type)
	{
		case SEGMENT_COMMON_CATHODE: break;
		case SEGMENT_COMMON_ANODE: break;
		default: segment->type = SEGMENT_COMMON_ANODE;
	}
}
/*----------------------------------------------------------------------------*/
void seven_show(seven_seg_t *segment,int number)
{
	volatile unsigned int loop;
	for(loop=0;loop<SEGMENT_COUNT;loop++)
	{
		if(segment->pattern[number*SEGMENT_COUNT+loop]!=segment->type)
			gpio_set(segment->gpio_num[loop]);
		else
			gpio_clr(segment->gpio_num[loop]);
	}
}
/*----------------------------------------------------------------------------*/
void seven_decp(seven_seg_t *segment, int dp)
{
	if(segment->gpio_dp<0) return;
	if(dp!=segment->type) gpio_set(segment->gpio_dp);
	else gpio_clr(segment->gpio_dp);
}
/*----------------------------------------------------------------------------*/
void seven_full(seven_seg_t *segment,int number,int dp)
{
	seven_show(segment,number);
	seven_decp(segment,dp);
}
/*----------------------------------------------------------------------------*/
