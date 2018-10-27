/*----------------------------------------------------------------------------*/
#ifndef __MY1SPIH__
#define __MY1SPIH__
/*----------------------------------------------------------------------------*/
/** SPI0 @ALTF0 */
#define SPI0_SCLK 11
#define SPI0_MOSI 10
#define SPI0_MISO 9
#define SPI0_CE0N 8
#define SPI0_CE1N 7
#define SPI_SCLK SPI0_SCLK
#define SPI_MOSI SPI0_MOSI
#define SPI_MISO SPI0_MISO
#define SPI_CE0N SPI0_CE0N
#define SPI_CE1N SPI0_CE1N
/*----------------------------------------------------------------------------*/
#define SPI_CLK_DIVIDE_MASK 0xFFFF
#define SPI_CLK_DIVIDE_DEFAULT 0
/*----------------------------------------------------------------------------*/
#define SPI_SELECT_0 0x01
#define SPI_SELECT_1 0x02
#define SPI_SELECT_DEFAULT SPI_SELECT_0
/*----------------------------------------------------------------------------*/
#define SPI_ACTIVATE 1
#define SPI_DEACTIVATE 0
/*----------------------------------------------------------------------------*/
void spi_init(int clk_divide);
void spi_select(unsigned int which);
void spi_activate(unsigned int enable);
unsigned int spi_transfer(unsigned int data);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
