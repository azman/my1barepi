#define GPIO_BASE 0x20200000
#define GPIO_FSEL 0x00
#define GPIO_FSET 0x07
#define GPIO_FCLR 0x0A
#define GPIO_ACT_LED 47

/**  needs to be global, coz local needs stack => stack pointer! */
unsigned int *gpio, loop;

void main(void)
{
	/** point to gpio access register */
	gpio = (unsigned int*) GPIO_BASE;
	/** configure gpio as output */
	gpio[GPIO_FSEL+(GPIO_ACT_LED/10)] = 1 << (GPIO_ACT_LED%10)*3;
	/** main loop */
	while(1)
	{
		/** clear pin - on led! */
		gpio[GPIO_FCLR+(GPIO_ACT_LED/32)] = 1 << (GPIO_ACT_LED%32);
		/** delay a bit to allow us see the light! */
		for(loop=0;loop<0x3F0000;loop++);
		/** set pin - off led! */
		gpio[GPIO_FSET+(GPIO_ACT_LED/32)] = 1 << (GPIO_ACT_LED%32);
		/** delay a bit to allow us see the blink! */
		for(loop=0;loop<0x3F0000;loop++);
	}
}
