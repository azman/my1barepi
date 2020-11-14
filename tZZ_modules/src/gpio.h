/*----------------------------------------------------------------------------*/
#ifndef __MY1GPIO_H__
#define __MY1GPIO_H__
/*----------------------------------------------------------------------------*/
#include "raspi.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
/* on pi, physical gpio pins are gpio2 - gpio27 (26 total) */
#define GPIO_COUNT 54
/*----------------------------------------------------------------------------*/
#define GPIO_INPUT  0x00
#define GPIO_OUTPUT 0x01
#define GPIO_ALTF5  0x02
#define GPIO_ALTF4  0x03
#define GPIO_ALTF0  0x04
#define GPIO_ALTF1  0x05
#define GPIO_ALTF2  0x06
#define GPIO_ALTF3  0x07
/*----------------------------------------------------------------------------*/
#define GPIO_PULL_NONE 0x00
#define GPIO_PULL_DOWN 0x01
#define GPIO_PULL_UP   0x02
#define GPIO_PULL_MASK 0x03
/*----------------------------------------------------------------------------*/
#define GPIO_EVENT_EDGER 0x01
#define GPIO_EVENT_EDGEF 0x02
#define GPIO_EVENT_LVLHI 0x04
#define GPIO_EVENT_LVLLO 0x08
#define GPIO_EVENT_AEDGR 0x10
#define GPIO_EVENT_AEDGF 0x20
/*----------------------------------------------------------------------------*/
void gpio_config(int gpio_num, int gpio_sel);
void gpio_set(int gpio_num);
void gpio_clr(int gpio_num);
void gpio_write(int gpio_num, unsigned int value);
unsigned int gpio_read(int gpio_num);
void gpio_toggle(int gpio_num);
void gpio_pull(int gpio_num, int pull_dir);
/*----------------------------------------------------------------------------*/
/** GPIO_DATA => 20-27 : LSB-MSB */
void gpio_init_data(int gpio_sel); /** select GPIO_INPUT or GPIO_OUTPUT */
void gpio_put_data(unsigned int data);
unsigned int gpio_get_data(void);
/*----------------------------------------------------------------------------*/
void gpio_setevent(int gpio_num,int events); /** enable gpio events detection */
void gpio_rstevent(int gpio_num); /** clears event status */
unsigned int gpio_chkevent(int gpio_num); /** check event status */
/*----------------------------------------------------------------------------*/
#endif /* __MY1GPIO_H__ */
/*----------------------------------------------------------------------------*/
