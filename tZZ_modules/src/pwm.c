/*----------------------------------------------------------------------------*/
/**
 * - pwm controller
**/
/*----------------------------------------------------------------------------*/
#include "pwm.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define PWM_BASE (PMAP_BASE|PWM_OFFSET)
/*----------------------------------------------------------------------------*/
#define PWMCTL_OFFSET (PWM_BASE+0x00)
/* STA reg is basically fifo status? */
#define PWMSTA_OFFSET (PWM_BASE+0x04)
/*----------------------------------------------------------------------------*/
/* pwm0 channel 1? - @gpio12 (altf0) @gpio18 (altf5) */
#define GPIO_PWM0 18
#define PWM_RNG1_OFFSET (PWM_BASE+0x10)
#define PWM_DAT1_OFFSET (PWM_BASE+0x14)
/* pwm1 channel 2? - @gpio13 (altf0) @gpio19 (altf5) */
#define GPIO_PWM1 19
#define PWM_RNG2_OFFSET (PWM_BASE+0x20)
#define PWM_DAT2_OFFSET (PWM_BASE+0x24)
/*----------------------------------------------------------------------------*/
/** CTL for channel 1 (pwm0) */
/* pwm sub-mode: 0=pwm 1=ms */
#define PWMCH1_MSEN 0x0080
/* flag to use fifo (instead of reg) */
#define PWMCH1_USEF 0x0020
/* polarity: 0=normal 1=invert */
#define PWMCH1_POLA 0x0010
/* silence bit */
#define PWMCH1_SBIT 0x0008
/* repeat last (fifo) 0:interrupt 1:repeat last until not empty */
#define PWMCH1_RPTL 0x0004
/* mode: 0=pwm 1=serializer */
#define PWMCH1_MODE 0x0002
#define PWMCH1_PWEN 0x0001
/** CTL for channel 2 (pwm1) */
#define PWMCH2_MSEN 0x8000
#define PWMCH2_USEF 0x2000
#define PWMCH2_POLA 0x1000
#define PWMCH2_SBIT 0x0800
#define PWMCH2_RPTL 0x0400
#define PWMCH2_MODE 0x0200
#define PWMCH2_PWEN 0x0100
/*----------------------------------------------------------------------------*/
/** clock manager: should put in its own module? for now, just dump it here! */
#define CM_BASE (PMAP_BASE|CLKMAN_OFFSET)
#define CM_GP0CTL (CM_BASE+0x70)
#define CM_GP0DIV (CM_BASE+0x74)
#define CM_PWMCTL (CM_BASE+0xA0)
#define CM_PWMDIV (CM_BASE+0xA4)
/*----------------------------------------------------------------------------*/
#define CM_PASSWORD 0x5A000000
#define CM_PASSWORD_MASK 0xFF000000
/** cm ctl */
#define CM_CLKGEN_BUSY 0x80
#define CM_CLKGEN_ENAB 0x10
#define CM_CLKGEN_SRC_MASK 0x0F
#define CM_CLKGEN_SRC_OSC 0x01
/** cm div */
#define CM_CLKDIVI_MASK 0x00FFF000
#define CM_CLKDIVF_MASK 0x00000FFF
/*----------------------------------------------------------------------------*/
void pwm_init(unsigned int chan, unsigned int divi, unsigned int divf)
{
	unsigned int temp = PWMCH1_MSEN;
	switch (chan)
	{
		case 0: gpio_config(GPIO_PWM0,GPIO_ALTF5); break;
		default:
		case 1: gpio_config(GPIO_PWM1,GPIO_ALTF5); temp <<= 8; break;
	}
	put32(PWMCTL_OFFSET,get32(PWMCTL_OFFSET)|temp);
	/* stop clkgen */
	temp = getbit32(CM_PWMCTL,~CM_PASSWORD_MASK);
	temp &= CM_CLKGEN_ENAB;
	put32(CM_PWMCTL,temp|CM_PASSWORD);
	while (getbit32(CM_PWMCTL,CM_CLKGEN_BUSY));
	/* setup clock division values */
	put32(CM_PWMDIV,(CM_PASSWORD|(divi&0xfff)<<12|(divf&0xfff)));
	/* start clkgen */
	temp = getbit32(CM_PWMCTL,~CM_PASSWORD_MASK);
	temp &= ~CM_CLKGEN_SRC_MASK;
	temp |= CM_CLKGEN_SRC_OSC;
	put32(CM_PWMCTL,temp|CM_PASSWORD);
	/* enable in a separate write? */
	temp |= CM_CLKGEN_ENAB;
	put32(CM_PWMCTL,temp|CM_PASSWORD);
	while (!getbit32(CM_PWMCTL,CM_CLKGEN_BUSY));
}
/*----------------------------------------------------------------------------*/
void pwm_exec(unsigned int chan)
{
	unsigned int temp = PWMCH1_PWEN;
	switch (chan)
	{
		case 0: break;
		default: case 1: temp <<= 8; break;
	}
	setbit32(PWMCTL_OFFSET,temp);
}
/*----------------------------------------------------------------------------*/
void pwm_stop(unsigned int chan)
{
	unsigned int temp = PWMCH1_PWEN;
	switch (chan)
	{
		case 0: break;
		default: case 1: temp <<= 8; break;
	}
	clrbit32(PWMCTL_OFFSET,temp);
}
/*----------------------------------------------------------------------------*/
void pwm_prep_data(unsigned int chan, unsigned int data)
{
	switch (chan)
	{
		case 0: put32(PWM_DAT1_OFFSET,data); break;
		default:
		case 1: put32(PWM_DAT2_OFFSET,data); break;
	}
}
/*----------------------------------------------------------------------------*/
void pwm_prep_full(unsigned int chan, unsigned int full)
{
	switch (chan)
	{
		case 0: put32(PWM_RNG1_OFFSET,full); break;
		default:
		case 1: put32(PWM_RNG2_OFFSET,full); break;
	}
}
/*----------------------------------------------------------------------------*/
unsigned int pwm_curr_data(unsigned int chan)
{
	switch (chan)
	{
		case 0: chan = PWM_DAT1_OFFSET; break;
		default:
		case 1: chan = PWM_DAT2_OFFSET; break;
	}
	return get32(chan);
}
/*----------------------------------------------------------------------------*/
unsigned int pwm_curr_full(unsigned int chan)
{
	switch (chan)
	{
		case 0: chan = PWM_RNG1_OFFSET; break;
		default:
		case 1: chan = PWM_RNG2_OFFSET; break;
	}
	return get32(chan);
}
/*----------------------------------------------------------------------------*/
