/*----------------------------------------------------------------------------*/
#include "sdext.h"
#include "timer.h"
#include "spi.h"
/*----------------------------------------------------------------------------*/
#define SDCARD_MMC_MASK 0xC0
#define SDCARD_MMC_CMD_ 0x40
/*----------------------------------------------------------------------------*/
#define SDCARD_RESP_WAIT_STEP 100
/*----------------------------------------------------------------------------*/
#define RESP_R1_IDLE_STATE  0x01
#define RESP_R1_ERASE_RESET 0x02
#define RESP_R1_ILLEGAL_CMD 0x04
#define RESP_R1_CMD_CRC_ERR 0x08
#define RESP_R1_ERASESQ_ERR 0x10
#define RESP_R1_ADDRESS_ERR 0x20
#define RESP_R1_PARAM_ERROR 0x40
/*----------------------------------------------------------------------------*/
#define SPI_SELECT_00 SPI_SELECT_0
#define SPI_SELECT_SD SPI_SELECT_1
/*----------------------------------------------------------------------------*/
void sdext_init(void)
{
	int loop;
	/* sdc/mmc init flow (spi) */
	timer_wait(1000); /* wait at least 1ms */
	/* send 74 dummy clock cycles? 10 x 8-bit data?*/
	spi_select(SPI_SELECT_00); /* CS needs to be DESELECTED for sd spi mode! */
	spi_activate(SPI_ACTIVATE);
	for (loop=0;loop<10;loop++) {
		spi_transfer(SDCARD_DUMMY_DATA);
	}
	spi_activate(SPI_DEACTIVATE);
	spi_select(SPI_SELECT_SD);
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_command(int cmd, unsigned int arg, int crc)
{
	unsigned int res, cnt = SDCARD_RESP_WAIT_STEP;
	spi_activate(SPI_ACTIVATE);
	spi_transfer(cmd|SDCARD_MMC_CMD_);
	spi_transfer((arg>>24)&0xff);
	spi_transfer((arg>>16)&0xff);
	spi_transfer((arg>>8)&0xff);
	spi_transfer((arg)&0xff);
	spi_transfer(crc);
	do
	{
		/* wait card to be ready  */
		if ((res=spi_transfer(SDCARD_DUMMY_DATA))!=SDCARD_RESP_INVALID)
			break;
	}
	while(--cnt>0);
	spi_activate(SPI_DEACTIVATE);
	return res;
}
/*----------------------------------------------------------------------------*/
void sdext_doflush(int count,unsigned char *pbuff)
{
	int loop, test;
	spi_activate(SPI_ACTIVATE);
	for (loop=0;loop<count;loop++)
	{
		test = (int) spi_transfer(SDCARD_DUMMY_DATA);
		if (pbuff) pbuff[loop] = (unsigned char) (test&0xff);
	}
	spi_activate(SPI_DEACTIVATE);
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_idle(void)
{
	unsigned int card;
	/* GO_IDLE_STATE - software reset */
	card = sdext_command(SDCARD_CMD00,SDCARD_ARG_NONE_,SDCARD_CMD00_CRC);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	return card;
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_cmd8(unsigned char* buffer)
{
	unsigned int card;
	/* SEND_IF_COND - check vers? */
	card = sdext_command(SDCARD_CMD08,SDCARD_CMD08_ARG,SDCARD_CMD08_CRC);
	if (card==SDCARD_RESP_R1_IDLE)
		sdext_doflush(SDCARD_FLUSH_R7,buffer);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	return card;
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_cmd1(void)
{
	unsigned int card;
	/* SEND_OP_COND - initialization */
	card = sdext_command(SDCARD_CMD01,SDCARD_ARG_NONE_,SDCARD_CMD01_CRC);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	return card;
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_acmd41(void)
{
	unsigned int card;
	/* SD_SEND_OP_COND - set host capacity support HSC */
	card = sdext_command(SDCARD_APPCMD,SDCARD_ARG_NONE_,SDCARD_CMD55_CRC);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	if (card!=SDCARD_RESP_R1_IDLE) return card|SDCARD_ERROR_CMD55;
	card = sdext_command(SDCARD_CMD41,SDCARD_CMD41_ARG,SDCARD_CMD41_CRC);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	if (card==SDCARD_RESP_ILLEGAL) card |= SDCARD_ERROR_ACMD41;
	return card;
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_disable_crc(void)
{
	unsigned int card;
	/* CRC_ON_OFF - crc option switch  send ARG=0x00000001 to turn ON */
	card = sdext_command(SDCARD_CMD59,SDCARD_ARG_NONE_,SDCARD_DUMMY_CRC);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	return card;
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_blocksize(void)
{
	unsigned int card;
	card = sdext_command(SDCARD_CMD16,SDCARD_SECTOR_SIZE,SDCARD_DUMMY_CRC);
	sdext_doflush(SDCARD_FLUSH_R1,0x0);
	return card;
}
/*----------------------------------------------------------------------------*/
unsigned int sdext_read_block(unsigned int sector, unsigned char* buffer)
{
	int loop;
	unsigned int res, cnt = SDCARD_RESP_WAIT_STEP;
	do
	{
		/* sector size = 512, checksum should already be disabled? */
		res = sdext_command(SDCARD_RDBLOCK,sector<<9,SDCARD_DUMMY_CRC);
		sdext_doflush(SDCARD_FLUSH_R1,0x0);
		if (res==0x00) break;
	}
	while(--cnt>0);
	if (!cnt) return res;
	/* wait?? */
	spi_activate(SPI_ACTIVATE);
	cnt = SDCARD_RESP_WAIT_STEP;
	do
	{
		if ((res=spi_transfer(SDCARD_DUMMY_DATA))==0xFE) break;
	}
	while(--cnt>0);
	if (!cnt) return res;
	/* get the data */
	for (loop=0;loop<SDCARD_SECTOR_SIZE;loop++)
		buffer[loop] = spi_transfer(SDCARD_DUMMY_DATA);
	/* 16-bit dummy crc? */
	spi_transfer(SDCARD_DUMMY_CRC);
	spi_transfer(SDCARD_DUMMY_CRC);
	spi_activate(SPI_DEACTIVATE);
	return 0;
}
/*----------------------------------------------------------------------------*/
