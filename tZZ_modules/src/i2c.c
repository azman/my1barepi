/*----------------------------------------------------------------------------*/
/**
 * - for accessing i2c
 *   = using bit-banging technique
**/
/*----------------------------------------------------------------------------*/
#include "i2c.h"
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
static int i2c_sda, i2c_scl, i2c_stop;
static unsigned int i2c_wait, i2c_free, i2c_last;
/*----------------------------------------------------------------------------*/
/** routine i2c to write out start marker */
void i2c_do_start(void)
{
	/* enforce bus free time */
	while ((timer_read()-i2c_last)<i2c_free);
	/* now we are driving! master! */
	gpio_config(i2c_scl,GPIO_OUTPUT);
	gpio_config(i2c_sda,GPIO_OUTPUT);
	gpio_set(i2c_sda);
	timer_wait(i2c_wait);
	gpio_set(i2c_scl);
	timer_wait(i2c_wait); /* start condition setup time */
	gpio_clr(i2c_sda);
	timer_wait(i2c_wait); /* start condition hold time */
	gpio_clr(i2c_scl);
}
/*----------------------------------------------------------------------------*/
/** routine i2c to write out stop marker */
void i2c_do_stop(void)
{
	gpio_clr(i2c_sda);
	timer_wait(i2c_wait);
	gpio_set(i2c_scl);
	/* should check clock stretching? in case slave pull scl low! */
	timer_wait(i2c_wait); /* stop condition setup time */
	gpio_set(i2c_sda);
	/* no hold time for stop condition? :p */
	timer_wait(i2c_wait);
	/* stop driving! release lines */
	gpio_config(i2c_sda,GPIO_INPUT);
	gpio_config(i2c_scl,GPIO_INPUT);
	/* to check bus free time */
	i2c_last = timer_read();
}
/*----------------------------------------------------------------------------*/
/** routine i2c to write 1 bit */
void i2c_do_write_bit(int data)
{
	if (data) gpio_set(i2c_sda);
	else gpio_clr(i2c_sda);
	timer_wait(i2c_wait);
	gpio_set(i2c_scl);
	timer_wait(i2c_wait);
	/* should check clock stretching? in case slave pull scl low! */
	gpio_clr(i2c_scl);
}
/*----------------------------------------------------------------------------*/
/** routine i2c to read 1 bit */
int i2c_do_read_bit(void)
{
	int data = 0;
	/* release the line for slave */
	gpio_set(i2c_sda);
	gpio_config(i2c_sda,GPIO_INPUT);
	timer_wait(i2c_wait);
	gpio_set(i2c_scl);
	/* should check clock stretching? in case slave pull scl low! */
	timer_wait(i2c_wait);
	if (gpio_read(i2c_sda)) data = 1;
	gpio_clr(i2c_scl);
	/* retake the line */
	gpio_config(i2c_sda,GPIO_OUTPUT);
	return data;
}
/*----------------------------------------------------------------------------*/
/** routine i2c to write 1 byte out */
int i2c_do_write_byte(int data)
{
	int loop, mask = 0x80;
	for (loop=0;loop<8;loop++)
	{
		i2c_do_write_bit(data&mask);
		mask >>= 1;
	}
	return i2c_do_read_bit();
}
/*----------------------------------------------------------------------------*/
/** routine i2c to read 1 byte in (acknowledge optional) */
int i2c_do_read_byte(int ack)
{
	int loop, mask = 0x80, data = 0x0;
	for (loop=0;loop<8;loop++)
	{
		if (i2c_do_read_bit())
			data |= mask;
		mask >>= 1;
	}
	i2c_do_write_bit(ack?0:1);
	return data;
}
/*----------------------------------------------------------------------------*/
void i2c_init(int sda_gpio, int scl_gpio)
{
	/* assign gpio */
	i2c_sda = sda_gpio;
	i2c_scl = scl_gpio;
	/* set pull-up on pins */
	gpio_pull(i2c_sda,GPIO_PULL_UP);
	gpio_pull(i2c_scl,GPIO_PULL_UP);
	/* setup i2c sda1/scl1 as input (tri-state) - in case multiple masters */
	gpio_config(i2c_sda,GPIO_INPUT);
	gpio_config(i2c_scl,GPIO_INPUT);
	/* just prepare this */
	gpio_set(i2c_sda);
	gpio_set(i2c_scl);
	/* default parameters */
	i2c_wait = I2C_WAIT_DEFAULT;
	i2c_free = I2C_WAIT_DEFAULT;
	i2c_stop = 0;
	/* to check bus free time */
	i2c_last = timer_read();
}
/*----------------------------------------------------------------------------*/
void i2c_set_wait_time(unsigned int wait_time)
{
	i2c_wait = wait_time;
}
/*----------------------------------------------------------------------------*/
void i2c_set_free_time(unsigned int free_time)
{
	i2c_free = free_time;
}
/*----------------------------------------------------------------------------*/
void i2c_set_stop_read(int enable)
{
	i2c_stop = enable ? 1 : 0;
}
/*----------------------------------------------------------------------------*/
void i2c_putb(int addr, int regs, int data)
{
	addr <<= 1;
	i2c_do_start();
	i2c_do_write_byte(addr);
	i2c_do_write_byte(regs);
	i2c_do_write_byte(data);
	i2c_do_stop();
}
/*----------------------------------------------------------------------------*/
int i2c_getb(int addr, int regs)
{
	int data;
	addr <<= 1;
	i2c_do_start();
	i2c_do_write_byte(addr);
	i2c_do_write_byte(regs);
	if (i2c_stop) i2c_do_stop();
	i2c_do_start();
	i2c_do_write_byte(addr|0x01); /* activate read bit */
	data = i2c_do_read_byte(0);
	i2c_do_stop();
	return data & 0xff;
}
/*----------------------------------------------------------------------------*/
int i2c_puts(int addr, int regs, unsigned char* pdat, int size)
{
	int loop, test = 0;
	addr <<= 1;
	i2c_do_start();
	test |= i2c_do_write_byte(addr);
	test |= i2c_do_write_byte(regs);
	for (loop=0;loop<size;loop++)
		test |= i2c_do_write_byte((int)pdat[loop]);
	i2c_do_stop();
	return test;
}
/*----------------------------------------------------------------------------*/
int i2c_gets(int addr, int regs, unsigned char* pdat, int size)
{
	int loop, test = 0;
	addr <<= 1;
	i2c_do_start();
	test |= i2c_do_write_byte(addr);
	test |= i2c_do_write_byte(regs);
	if (i2c_stop) i2c_do_stop();
	i2c_do_start();
	test |= i2c_do_write_byte(addr|0x01); /* activate read bit */
	for (loop=0;loop<size-1;loop++)
		pdat[loop] = (unsigned char)i2c_do_read_byte(1);
	pdat[loop] = i2c_do_read_byte(0);
	i2c_do_stop();
	return test;
}
/*----------------------------------------------------------------------------*/
