/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"
#include "utils.h"
#include "font.h"
#include "spi.h"
/*----------------------------------------------------------------------------*/
#define ERROR_LED 47
#define ERROR_SDCARD_BLINK 3
/*----------------------------------------------------------------------------*/
#define SPI_CLK_DIVIDE_TEST 1024
/*
  512      488 kHz
  1024      244 kHz
*/
/*----------------------------------------------------------------------------*/
#define SDCARD_CMD00 0x00
#define SDCARD_CMD00_CRC 0x95
#define SDCARD_CMD01 0x01
#define SDCARD_CMD01_CRC 0xF9
#define SDCARD_CMD08 0x08
#define SDCARD_CMD08_ARG 0x000001AA
#define SDCARD_CMD08_CRC 0x87
#define SDCARD_CMD12 0x0C
#define SDCARD_CMD16 0x10
#define SDCARD_CMD17 0x11
#define SDCARD_CMD41 0x29
/** CMD41 arg/crc assumes HCS (high capacity?), else 0x00000000 & 0xE5 */
#define SDCARD_CMD41_ARG 0x40000000
#define SDCARD_CMD41_CRC 0x77
#define SDCARD_CMD55 0x37
#define SDCARD_CMD55_CRC 0x65
#define SDCARD_CMD59 0x3B
/*----------------------------------------------------------------------------*/
#define SDCARD_SWRESET SDCARD_CMD00
#define SDCARD_DOINIT SDCARD_CMD01
#define SDCARD_STOPTX SDCARD_CMD12
#define SDCARD_CHBSIZE SDCARD_CMD16
#define SDCARD_RDBLOCK SDCARD_CMD17
#define SDCARD_APPCMD SDCARD_CMD55
#define SDCARD_NOCRC SDCARD_CMD59
/*----------------------------------------------------------------------------*/
#define SDCARD_ARG_NONE_ 0x00000000
#define SDCARD_DUMMY_DATA 0xFF
#define SDCARD_DUMMY_CRC SDCARD_DUMMY_DATA
/*----------------------------------------------------------------------------*/
#define SDCARD_MMC_MASK 0xC0
#define SDCARD_MMC_CMD_ 0x40
#define SDCARD_SECTOR_SIZE 512
/*----------------------------------------------------------------------------*/
#define SDCARD_RESP_SUCCESS 0x00
#define SDCARD_RESP_R1_IDLE 0x01
#define SDCARD_RESP_ILLEGAL 0x05
#define SDCARD_RESP_INVALID 0xFF
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
#define SDCARD_FLUSH_R1 1
#define SDCARD_FLUSH_R7 4
/*----------------------------------------------------------------------------*/
void sdcard_init(void)
{
	int loop;
	/* sdc/mmc init flow (spi) */
	timer_wait(1000); /* wait at least 1ms */
	/* send 74 dummy clock cycles? 10 x 8-bit data?*/
	spi_activate(SPI_ACTIVATE);
	for (loop=0;loop<10;loop++) {
		spi_transfer(SDCARD_DUMMY_DATA);
	}
	spi_activate(SPI_DEACTIVATE);
}
/*----------------------------------------------------------------------------*/
unsigned int sdcard_command(int cmd, unsigned int arg, int crc)
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
void sdcard_doflush(int count,unsigned char *pbuff)
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
unsigned int sdcard_read_block(unsigned int sector, unsigned char* buffer)
{
	int loop;
	unsigned int res, cnt = SDCARD_RESP_WAIT_STEP;
	do
	{
		/* sector size = 512, checksum should already be disabled? */
		res = sdcard_command(SDCARD_RDBLOCK,sector<<9,SDCARD_DUMMY_CRC);
		sdcard_doflush(SDCARD_FLUSH_R1,0x0);
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
void error_blink(int count)
{
	int loop, flip = 0;
	while(1) /* error condition, hangout here! */
	{
		for (loop=0;loop<count;loop++)
		{
			if (flip) gpio_set(ERROR_LED);
			timer_wait(TIMER_S/2);
			gpio_clr(ERROR_LED);
			timer_wait(TIMER_S/2);
		}
		flip = !flip;
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	unsigned int card;
	unsigned char sector[SDCARD_SECTOR_SIZE];
	int loop;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	/** initialize timer */
	timer_init();
	/** initialize mailbox */
	mailbox_init();
	/** initialize video */
	video_init(VIDEO_RES_VGA);
	/* setup screen */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/* say something... */
	video_text_string("--------------------\n");
	video_text_string("SD Card Reader Test!\n");
	video_text_string("--------------------\n\n");
	/** initialize spi */
	spi_init(SPI_CLK_DIVIDE_TEST);
	spi_select(SPI_SELECT_1);
	/* initialize sd card */
	video_text_string("Initializing SDCARD... ");
	sdcard_init();
	video_text_string("done.\n");
	/* software reset */
	video_text_string("Sending idle command... ");
	card = sdcard_command(SDCARD_CMD00,SDCARD_ARG_NONE_,SDCARD_CMD00_CRC);
	sdcard_doflush(SDCARD_FLUSH_R1,0x0);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	if (card!=SDCARD_RESP_R1_IDLE) error_blink(ERROR_SDCARD_BLINK);
	/* find out about card? */
	video_text_string("Check SD card... ");
	card = sdcard_command(SDCARD_CMD08,SDCARD_CMD08_ARG,SDCARD_CMD08_CRC);
	sdcard_doflush(SDCARD_FLUSH_R7,sector);
	sdcard_doflush(SDCARD_FLUSH_R1,0x0);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(")=>(0x");
	video_text_hexbyte(sector[0]);
	for (loop=1;loop<SDCARD_FLUSH_R7;loop++)
	{
		video_text_string(",0x");
		video_text_hexbyte(sector[loop]);
	}
	video_text_string(").\n");
	if (card==SDCARD_RESP_ILLEGAL)
	{
		/* sd card version 1 OR mmc v3? */
		video_text_string("  => SD Card version 1 detected.\n");
		video_text_string("Command OP COND... "); loop = 0;
		do
		{
			card = sdcard_command(SDCARD_CMD01,
				SDCARD_ARG_NONE_,SDCARD_CMD01_CRC);
			sdcard_doflush(SDCARD_FLUSH_R1,0x0);
			loop++;
		}
		while (card!=SDCARD_RESP_SUCCESS);
		video_text_string("done (0x");
		video_text_hexbyte((unsigned char)card);
		video_text_string(",loop=");
		video_text_integer(loop);
		video_text_string(").\n");
	}
	else if (card==SDCARD_RESP_R1_IDLE)
	{
		/* sd card version 2, maybe? */
		if (sector[0]==0x00&&sector[1]==0x00&&
				sector[2]==0x01&&sector[3]==0xAA)
		{
			video_text_string("  => SD Card version 2 detected.\n");
			/* sending app cmd */

			video_text_string("Command ACMD41... "); loop = 0;
			do
			{
				card = sdcard_command(SDCARD_APPCMD,
					SDCARD_ARG_NONE_,SDCARD_CMD55_CRC);
				sdcard_doflush(SDCARD_FLUSH_R1,0x0);
				if (card!=SDCARD_RESP_R1_IDLE)
				{
					video_text_string("Cannot send CMD55? Abort.\n");
					error_blink(ERROR_SDCARD_BLINK);
				}
				card = sdcard_command(SDCARD_CMD41,
					SDCARD_CMD41_ARG,SDCARD_CMD41_CRC);
				sdcard_doflush(SDCARD_FLUSH_R1,0x0);
				if (card==SDCARD_RESP_ILLEGAL)
				{
					video_text_string("Cannot send CMD41? Abort.\n");
					error_blink(ERROR_SDCARD_BLINK);
				}
				loop++;
			}
			while (card!=SDCARD_RESP_SUCCESS);
			video_text_string("done (0x");
			video_text_hexbyte((unsigned char)card);
			video_text_string(",loop=");
			video_text_integer(loop);
			video_text_string(").\n");
		}
		else
		{
			video_text_string("Unknown card? Abort.\n");
			error_blink(ERROR_SDCARD_BLINK);
		}
	}
	else
	{
		video_text_string("Unknown response? Abort.\n");
		error_blink(ERROR_SDCARD_BLINK);
	}
	/* turn off crc? */
	video_text_string("Command CRC off... ");
	card = sdcard_command(SDCARD_NOCRC,SDCARD_ARG_NONE_,SDCARD_DUMMY_CRC);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	sdcard_doflush(SDCARD_FLUSH_R1,0x0);
	/* set block length @ sector size */
	video_text_string("Command set block len... ");
	card = sdcard_command(SDCARD_CHBSIZE,SDCARD_ARG_NONE_,SDCARD_DUMMY_CRC);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	sdcard_doflush(SDCARD_FLUSH_R1,0x0);
	/* change speed - can get up to 12MHz? */
	/** initialize spi */
	video_text_string("Reading first sector... ");
	card = sdcard_read_block(0,sector);
	video_text_string("done. (");
	video_text_hexuint(card);
	video_text_string(")\n\n");
	for (loop=0;loop<8;loop++)
	{
		video_text_string("[0x");
		video_text_hexbyte(sector[loop]);
		video_text_string("]");
	}
	/** main loop */
	while(1) { }
}
/*----------------------------------------------------------------------------*/
