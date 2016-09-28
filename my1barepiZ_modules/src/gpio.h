/*----------------------------------------------------------------------------*/
#ifndef __MY1GPIOH__
#define __MY1GPIOH__
/*----------------------------------------------------------------------------*/
#ifdef RASPI2
#define GPIO_BASE 0x3F200000
#else
#define GPIO_BASE 0x20200000
#endif
/*----------------------------------------------------------------------------*/
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
void gpio_init(void);
void gpio_config(int gpio_num, int gpio_sel);
void gpio_set(int gpio_num);
void gpio_clr(int gpio_num);
void gpio_toggle(int gpio_num);
unsigned int gpio_read(int gpio_num);
void gpio_pull(int gpio_num, int pull_dir);
/*----------------------------------------------------------------------------*/
void gpio_setevent(int gpio_num,int events); /** enable gpio events detection */
void gpio_rstevent(int gpio_num); /** clears event status */
unsigned int gpio_chkevent(int gpio_num); /** check event status */
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
