/*----------------------------------------------------------------------------*/
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define GPIO_BASE 0x20200000
#define GPIO_FSEL 0x00
#define GPIO_FSET 0x07
#define GPIO_FCLR 0x0A
#define GPIO_FGET 0x0D
#define GPIO_FPUD 0x25
/*----------------------------------------------------------------------------*/
#define GPIO_SELECT_BITS 3
#define GPIO_SELECT 0x07
/*----------------------------------------------------------------------------*/
volatile unsigned int *gpio;
/*----------------------------------------------------------------------------*/
void gpio_init(void)
{
	gpio = (unsigned int*) GPIO_BASE;
}
/*----------------------------------------------------------------------------*/
void gpio_config(int gpio_num, int gpio_sel)
{
	unsigned int shift = (gpio_num%10)*GPIO_SELECT_BITS;
	unsigned int index = (gpio_num/10)+GPIO_FSEL;
	unsigned int mask = GPIO_SELECT << shift;
	unsigned int value = gpio_sel << shift;
	gpio[index] &= ~mask;
	gpio[index] |= value;
}
/*----------------------------------------------------------------------------*/
void gpio_set(int gpio_num)
{
	gpio[GPIO_FSET+(gpio_num/32)] = 1 << (gpio_num%32);
}
/*----------------------------------------------------------------------------*/
void gpio_clr(int gpio_num)
{
	gpio[GPIO_FCLR+(gpio_num/32)] = 1 << (gpio_num%32);
}
/*----------------------------------------------------------------------------*/
unsigned int gpio_read(int gpio_num)
{
	return gpio[GPIO_FGET+(gpio_num/32)] & (1<<(gpio_num%32));
}
/*----------------------------------------------------------------------------*/
#define GPIO_PULL_WAIT 150
/*----------------------------------------------------------------------------*/
void gpio_pull(int gpio_num, int pull_dir)
{
	unsigned int shift = (gpio_num%32);
	unsigned int index = (gpio_num/32)+1;
	unsigned int loop;
	gpio[GPIO_FPUD] = pull_dir & GPIO_PULL_MASK;
	for(loop=0;loop<GPIO_PULL_WAIT;loop++); /* setup time: 150 cycles? */
	gpio[GPIO_FPUD+index] = 1 << shift; /* enable ppud clock */
	for(loop=0;loop<GPIO_PULL_WAIT;loop++); /* hold time: 150 cycles? */
	gpio[GPIO_FPUD] = GPIO_PULL_NONE;
	gpio[GPIO_FPUD+index] = 0; /* disable ppud clock */
}
/*----------------------------------------------------------------------------*/
