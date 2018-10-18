/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define GPIO_BASE (PMAP_BASE|GPIO_OFFSET)
/*----------------------------------------------------------------------------*/
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define GPIO_FSEL (GPIO_BASE+0x00)
#define GPIO_FSET (GPIO_BASE+0x1C)
#define GPIO_FCLR (GPIO_BASE+0x28)
#define GPIO_FGET (GPIO_BASE+0x34)
#define GPIO_EVDS (GPIO_BASE+0x40)
#define GPIO_EREN (GPIO_BASE+0x4C)
#define GPIO_EFEN (GPIO_BASE+0x58)
#define GPIO_LHEN (GPIO_BASE+0x64)
#define GPIO_LLEN (GPIO_BASE+0x70)
#define GPIO_AREN (GPIO_BASE+0x7C)
#define GPIO_AFEN (GPIO_BASE+0x88)
#define GPIO_FPUD (GPIO_BASE+0x94)
/*----------------------------------------------------------------------------*/
#define GPIO_SELECT_BITS 3
#define GPIO_SELECT 0x07
/*----------------------------------------------------------------------------*/
#define GPIO_DATA_OFFSET 20
#define GPIO_DATA_DECADE (GPIO_BASE+(2<<2))
/** 0010_0100_1001_0010_0100_1001 */
#define GPIO_DATA_OUTPUT 0x00249249
#define GPIO_DATA_DOMASK 0x00FFFFFF
/*----------------------------------------------------------------------------*/
/* functions defined in boot*.s */
unsigned int get32(unsigned int);
void put32(unsigned int,unsigned int);
void loopd(unsigned int);
/*----------------------------------------------------------------------------*/
void gpio_init(void)
{
	/* nothing to do? will be deprecated? */
}
/*----------------------------------------------------------------------------*/
void gpio_config(int gpio_num, int gpio_sel)
{
	unsigned int raddr = GPIO_FSEL+((gpio_num/10)<<2);
	unsigned int shift = (gpio_num%10)*GPIO_SELECT_BITS;
	unsigned int value = gpio_sel << shift;
	unsigned int mask = GPIO_SELECT << shift;
	unsigned int data = get32(raddr);
	data &= ~mask;
	data |= value;
	put32(raddr,data);
}
/*----------------------------------------------------------------------------*/
void gpio_set(int gpio_num)
{
	put32(GPIO_FSET+((gpio_num/32)<<2),1<<(gpio_num%32));
}
/*----------------------------------------------------------------------------*/
void gpio_clr(int gpio_num)
{
	put32(GPIO_FCLR+((gpio_num/32)<<2),1<<(gpio_num%32));
}
/*----------------------------------------------------------------------------*/
void gpio_write(int gpio_num, int value)
{
	if(value) gpio_set(gpio_num);
	else gpio_clr(gpio_num);
}
/*----------------------------------------------------------------------------*/
unsigned int gpio_read(int gpio_num)
{
	return get32(GPIO_FGET+((gpio_num/32)<<2))&(1<<(gpio_num%32));
}
/*----------------------------------------------------------------------------*/
void gpio_toggle(int gpio_num)
{
	if(gpio_read(gpio_num)) gpio_clr(gpio_num);
	else gpio_set(gpio_num);
}
/*----------------------------------------------------------------------------*/
#define GPIO_PULL_WAIT 150
/*----------------------------------------------------------------------------*/
void gpio_pull(int gpio_num, int pull_dir)
{
	unsigned int shift = (gpio_num%32);
	unsigned int index = (gpio_num/32)+1;
	put32(GPIO_FPUD,pull_dir&GPIO_PULL_MASK);
	loopd(GPIO_PULL_WAIT); /* setup time: 150 cycles? */
	put32(GPIO_FPUD+(index<<2),1<<shift); /* enable ppud clock */
	loopd(GPIO_PULL_WAIT); /* hold time: 150 cycles? */
	put32(GPIO_FPUD,GPIO_PULL_NONE);
	put32(GPIO_FPUD+(index<<2),0); /* disable ppud clock */
}
/*----------------------------------------------------------------------------*/
void gpio_init_data(int gpio_sel)
{
	unsigned int conf = get32(GPIO_DATA_DECADE);
	conf &= ~GPIO_DATA_DOMASK;
	if(gpio_sel==GPIO_OUTPUT)
		conf |= GPIO_DATA_OUTPUT;
	put32(GPIO_DATA_DECADE,conf);
}
/*----------------------------------------------------------------------------*/
void gpio_put_data(unsigned int data)
{
	put32(GPIO_FSET,(data&0xff)<<GPIO_DATA_OFFSET);
	put32(GPIO_FCLR,(~data&0xff)<<GPIO_DATA_OFFSET);
}
/*----------------------------------------------------------------------------*/
unsigned int gpio_get_data(void)
{
	unsigned int data = get32(GPIO_FGET);
	return (data>>GPIO_DATA_OFFSET)&0xff;
}
/*----------------------------------------------------------------------------*/
void gpio_setevent(int gpio_num,int events)
{
	unsigned int shift = (gpio_num%32);
	unsigned int index = (gpio_num/32);
	unsigned int mask = 1 << shift;
	unsigned int data;
	/* clear by default, set only if requested */
	/* enable rising edge detect status */
	data = get32(GPIO_EREN+(index<<2));
	data &= ~mask;
	if(events&GPIO_EVENT_EDGER) data |= mask;
	put32((GPIO_EREN+(index<<2)),data);
	/* enable falling edge detect status */
	data = get32(GPIO_EFEN+(index<<2));
	data &= ~mask;
	if(events&GPIO_EVENT_EDGEF) data |= mask;
	put32((GPIO_EFEN+(index<<2)),data);
	/* enable high level detect status */
	data = get32(GPIO_LHEN+(index<<2));
	data &= ~mask;
	if(events&GPIO_EVENT_LVLHI) data |= mask;
	put32((GPIO_LHEN+(index<<2)),data);
	/* enable low level detect status */
	data = get32(GPIO_LLEN+(index<<2));
	data &= ~mask;
	if(events&GPIO_EVENT_LVLLO) data |= mask;
	put32((GPIO_LLEN+(index<<2)),data);
	/* enable asynchronous rising edge detect status */
	data = get32(GPIO_AREN+(index<<2));
	data &= ~mask;
	if(events&GPIO_EVENT_AEDGR) data |= mask;
	put32((GPIO_AREN+(index<<2)),data);
	/* enable asynchronous falling edge detect status */
	data = get32(GPIO_AFEN+(index<<2));
	data &= ~mask;
	if(events&GPIO_EVENT_AEDGF) data |= mask;
	put32((GPIO_AFEN+(index<<2)),data);
}
/*----------------------------------------------------------------------------*/
void gpio_rstevent(int gpio_num)
{
	put32(GPIO_EVDS+((gpio_num/32)<<2),1<<(gpio_num%32));
}
/*----------------------------------------------------------------------------*/
unsigned int gpio_chkevent(int gpio_num)
{
	return get32(GPIO_EVDS+((gpio_num/32)<<2));
}
/*----------------------------------------------------------------------------*/
