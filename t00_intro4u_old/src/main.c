#if defined RASPI3
#define GPIO_BASE 0x3F200000
/** v3 op freq is 1.2GHz (Cortex A53 quad core! => b+ @1.4GHz) */
#define COUNT_MAX 0x500000
#elif defined RASPI2
#define GPIO_BASE 0x3F200000
/** v2 op freq is 900MHz (Cortex A7 quad core!) */
#define COUNT_MAX 0x300000
#else
#define GPIO_BASE 0x20200000
/** v1 op freq is 700MHz (ARM11 single core!) */
#define COUNT_MAX 0x200000
#endif
#define GPIO_FSEL 0x00
#define GPIO_FSET 0x07
#define GPIO_FCLR 0x0A
/** GPIO number for ACT LED on R-Pi Model B+ v1.2 */
/** ACT LED on Pi B+ v1.2 is active high */
/** - was previously on 16, active low! */
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
