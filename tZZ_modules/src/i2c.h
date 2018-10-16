/*----------------------------------------------------------------------------*/
#ifndef __MY1I2CH__
#define __MY1I2CH__
/*----------------------------------------------------------------------------*/
#define I2C_SDA1_GPIO 2
#define I2C_SCL1_GPIO 3
#define I2C_SDA_GPIO I2C_SDA1_GPIO
#define I2C_SCL_GPIO I2C_SCL1_GPIO
/*----------------------------------------------------------------------------*/
/* for bit-banging i2c: blocking functions */
void i2c_init(int sda, int scl);
void i2c_putb(int addr, int regs, int data);
int i2c_getb(int addr, int regs);
int i2c_puts(int addr, int regs, int* pdat, int size);
int i2c_gets(int addr, int regs, int* pdat, int size);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
