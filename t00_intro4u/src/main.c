#if defined RASPI3
#define GPIO_BASE 0x3F200000
#define COUNT_MAX 0x080000
/** v3 op freq is 1.2GHz (quad core!) */
#elif defined RASPI2
#define GPIO_BASE 0x3F200000
#define COUNT_MAX 0x100000
/** v2 op freq is 900MHz (quad core!) */
#else
#define GPIO_BASE 0x20200000
#define COUNT_MAX 0x200000
#endif
#define GPIO_FSEL 0x00
#define GPIO_FSET 0x07
#define GPIO_FCLR 0x0A
/** GPIO number for ACT LED on R-Pi Model B+ v1.2 */
#define GPIO_ACT_LED 47

/**
 *
 * This can be added to the code to avoid using boot.s
 *
 * void main(void) __attribute__((naked));
 *
**/
void main(void)
{
	/** volatile coz -O2 compiler option would otherwise kill them? */
	volatile unsigned int *gpio, loop;
	/** point to gpio access register */
	gpio = (unsigned int*) GPIO_BASE;
	/** configure gpio as output */
	gpio[GPIO_FSEL+(GPIO_ACT_LED/10)] = 1 << (GPIO_ACT_LED%10)*3;
	/** main loop */
	while(1)
	{
		/** clear pin! */
		gpio[GPIO_FCLR+(GPIO_ACT_LED/32)] = 1 << (GPIO_ACT_LED%32);
		/** delay a bit to allow us see the blink! */
		for(loop=0;loop<COUNT_MAX;loop++);
		/** set pin! */
		gpio[GPIO_FSET+(GPIO_ACT_LED/32)] = 1 << (GPIO_ACT_LED%32);
		/** delay a bit to allow us see the blink! */
		for(loop=0;loop<COUNT_MAX;loop++);
	}
}
