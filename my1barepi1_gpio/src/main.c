/*----------------------------------------------------------------------------*/
/** GPIO MODULE BEGIN - THESE FUNCTIONS WILL BE MADE INTO A LIBRARY! */
/*----------------------------------------------------------------------------*/
#ifdef RASPI2
#define GPIO_BASE 0x3F200000
#else
#define GPIO_BASE 0x20200000
#endif
#define GPIO_FSEL 0x00
#define GPIO_FSET 0x07
#define GPIO_FCLR 0x0A
#define GPIO_FGET 0x0D
/*----------------------------------------------------------------------------*/
#define GPIO_SELECT_BITS 3
#define GPIO_SELECT 0x07
/*----------------------------------------------------------------------------*/
#define GPIO_INPUT  0x00
#define GPIO_OUTPUT 0x01
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
/** GPIO MODULE END */
/*----------------------------------------------------------------------------*/
#define MY_LED 2
#define MY_SWITCH 3
/*----------------------------------------------------------------------------*/
#define LED_ON gpio_clr
#define LED_OFF gpio_set
#define SWITCH_IS_OFF gpio_read
/*----------------------------------------------------------------------------*/
#ifdef RASPI2
#define COUNT_MAX 0x100000
#else
#define COUNT_MAX 0x200000
#endif
/*----------------------------------------------------------------------------*/
void main(void)
{
	volatile int loop; /** loop will be optimized OUT if NOT volatile */
	gpio_init();
	gpio_config(MY_LED,GPIO_OUTPUT);
	gpio_config(MY_SWITCH,GPIO_INPUT);
	LED_OFF(MY_LED);
	while(1)
	{
		if(SWITCH_IS_OFF(MY_SWITCH)) continue;
		LED_ON(MY_LED);
		for(loop=0;loop<COUNT_MAX;loop++);
		LED_OFF(MY_LED);
		for(loop=0;loop<COUNT_MAX;loop++);
	}
}
/*----------------------------------------------------------------------------*/
