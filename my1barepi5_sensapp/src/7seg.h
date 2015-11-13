/*----------------------------------------------------------------------------*/
#ifndef __SEVEN_H
#define __SEVEN_H
/*----------------------------------------------------------------------------*/
#define SEGMENT_COUNT 7
/*----------------------------------------------------------------------------*/
#define SEGMENT_DP_NC -1
/*----------------------------------------------------------------------------*/
typedef struct _seven_seg_t
{
	int *p_pattern;
	int *gpio_num;
	int gpio_dp;
} seven_seg_t;
/*----------------------------------------------------------------------------*/
void seven_init(seven_seg_t *segment,int *pattern,int *gpio,int gpio_dp);
void seven_show(seven_seg_t *segment,int number);
void seven_decp(seven_seg_t *segment,int dp);
void seven_full(seven_seg_t *segment,int number,int dp);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
