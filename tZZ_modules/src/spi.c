/*----------------------------------------------------------------------------*/
/**
 * - for accessing spi
 *   = this is using spi0 (spi-specific interface)
 *   = bcm2835 has 2 more mini-spi interfaces (spi1/spi2)
 *     $ part of auxiliary peripheral (along with mini-uart)
 *     $ NOT using these
**/
/*----------------------------------------------------------------------------*/
#include "spi.h"
#include "gpio.h"
/*----------------------------------------------------------------------------*/
#define SPI_BASE (PMAP_BASE|SPI0_OFFSET)
/*----------------------------------------------------------------------------*/
#define SPI_CS_REG   (SPI_BASE+0x00)
#define SPI_FIFO_REG (SPI_BASE+0x04)
#define SPI_CLK_REG  (SPI_BASE+0x08)
#define SPI_DLEN_REG (SPI_BASE+0x0C)
#define SPI_LTOH_REG (SPI_BASE+0x10)
#define SPI_DC_REG   (SPI_BASE+0x14)
/*----------------------------------------------------------------------------*/
/* control status bits */
#define SPI_CNTL_CSPOL2 0x00800000
#define SPI_CNTL_CSPOL1 0x00400000
#define SPI_CNTL_CSPOL0 0x00200000
#define SPI_STAT_RXFULL 0x00100000
#define SPI_STAT_RXREAD 0x00080000
#define SPI_STAT_TXDATA 0x00040000
#define SPI_STAT_RXDATA 0x00020000
#define SPI_STAT_TXDONE 0x00010000
#define SPI_CNTL_READEN 0x00001000
#define SPI_CNTL_ADCS   0x00000800
#define SPI_CNTL_INTRXR 0x00000400
#define SPI_CNTL_INTRDN 0x00000200
#define SPI_CNTL_DMA_EN 0x00000100
#define SPI_CNTL_TRXACT 0x00000080
#define SPI_CNTL_CSPOL  0x00000040
#define SPI_CNTL_CLMASK 0x00000030
#define SPI_CNTL_CLR_RX 0x00000020
#define SPI_CNTL_CLR_TX 0x00000010
#define SPI_CNTL_CPOL   0x00000008
#define SPI_CNTL_CPHA   0x00000004
#define SPI_CNTL_CSMASK 0x00000003
#define SPI_CNTL_CS0    0x00000000
#define SPI_CNTL_CS1    0x00000001
#define SPI_CNTL_CS2    0x00000002
/*----------------------------------------------------------------------------*/
static unsigned int spi_which = SPI_SELECT_DEFAULT;
/*----------------------------------------------------------------------------*/
void spi_init(int clk_divide)
{
	unsigned int data = SPI_CNTL_CLMASK; /* clear both rx/tx fifo */
	/* clear spi fifo */
	put32(SPI_CS_REG,data);
	/* set largest clock divider */
	clk_divide &= SPI_CLK_DIVIDE_MASK; /* 16-bit value */
	put32(SPI_CLK_REG,clk_divide); /** 0=65536, power of 2, rounded down */
	/* setup spi pins (ALTF0) */
	gpio_config(SPI_SCLK,GPIO_ALTF0);
	gpio_config(SPI_MOSI,GPIO_ALTF0);
	gpio_config(SPI_MISO,GPIO_ALTF0);
	gpio_config(SPI_CE0N,GPIO_ALTF0);
	gpio_config(SPI_CE1N,GPIO_ALTF0);
}
/*----------------------------------------------------------------------------*/
void spi_select(unsigned int which)
{
	switch (which)
	{
		case SPI_SELECT_0:
		case SPI_SELECT_1:
			spi_which = which;
			break;
		default:
			spi_which = SPI_SELECT_DEFAULT;
	}
}
/*----------------------------------------------------------------------------*/
void spi_activate(unsigned int enable)
{
	unsigned int data = SPI_CNTL_TRXACT;
	if (enable)
	{
		/* activate transfer on selected channel 0 or 1 */
		put32(SPI_CS_REG,data|(spi_which>>1));
	}
	else
	{
		/* de-activate transfer */
		put32(SPI_CS_REG,get32(SPI_CS_REG)&~SPI_CNTL_TRXACT);
	}
}
/*----------------------------------------------------------------------------*/
unsigned int spi_transfer(unsigned int data)
{
	/* wait if fifo is full */
	while (!(get32(SPI_CS_REG)&SPI_STAT_TXDATA));
	/* write a byte */
	put32(SPI_FIFO_REG,data&0xff);
	/* wait until done */
	while (!(get32(SPI_CS_REG)&SPI_STAT_TXDONE));
	/* should get a byte? */
	while (!(get32(SPI_CS_REG)&SPI_STAT_RXDATA));
	/* read a byte */
	return get32(SPI_FIFO_REG)&0xff;
}
/*----------------------------------------------------------------------------*/
