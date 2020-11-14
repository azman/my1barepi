/*----------------------------------------------------------------------------*/
#ifndef __MY1SEVEN_H__
#define __MY1SEVEN_H__
/*----------------------------------------------------------------------------*/
#define SEGMENT_COUNT 7
/*----------------------------------------------------------------------------*/
#define NOT_CONNECTED -1
/*----------------------------------------------------------------------------*/
#define COMMON_ANODE 0
#define COMMON_CATHODE 1
/*----------------------------------------------------------------------------*/
#define LED_ON 0
#define LED_OFF 1
/*----------------------------------------------------------------------------*/
typedef struct _seven_seg_t
{
	int *pattern;
	int gpio_num[SEGMENT_COUNT];
	int gpio_dp, type;
} seven_seg_t;
/*----------------------------------------------------------------------------*/
void seven_init(seven_seg_t *segment);
void seven_show(seven_seg_t *segment,int number);
void seven_decp(seven_seg_t *segment,int dp);
void seven_full(seven_seg_t *segment,int number,int dp);
/*----------------------------------------------------------------------------*/
#endif /* __MY1SEVEN_H__ */
/*----------------------------------------------------------------------------*/
