/*----------------------------------------------------------------------------*/
#ifndef __MY1GPIOH__
#define __MY1GPIOH__
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
extern volatile unsigned int *gpio;
/*----------------------------------------------------------------------------*/
void gpio_init(void);
void gpio_config(int gpio_num, int gpio_sel);
void gpio_set(int gpio_num);
void gpio_clr(int gpio_num);
unsigned int gpio_read(int gpio_num);
void gpio_pull(int gpio_num, int pull_dir);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
