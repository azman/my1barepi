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
#define ERROR_VIDEO_BLINK 2
#define ERROR_SDCARD_BLINK 3
/*----------------------------------------------------------------------------*/
#define SPI_CLK_DIVIDE_TEST 1024
/*
  512      488 kHz
  1024      244 kHz
*/
/*----------------------------------------------------------------------------*/
#define SDCARD_CMD00 0x00
#define SDCARD_CMD01 0x01
#define SDCARD_CMD08 0x08
#define SDCARD_CMD12 0x0C
#define SDCARD_CMD16 0x10
#define SDCARD_CMD17 0x11
#define SDCARD_CMD41 0x29
#define SDCARD_CMD55 0x37
#define SDCARD_CMD59 0x3B
/*----------------------------------------------------------------------------*/
#define SDCARD_SOFTWARE_RESET SDCARD_CMD00
#define SDCARD_INITIALIZE SDCARD_CMD01
#define SDCARD_STOP_TRANSMISSION SDCARD_CMD12
#define SDCARD_CHANGE_BLOCK_SIZE SDCARD_CMD16
#define SDCARD_READ_BLOCK SDCARD_CMD17
#define SDCARD_APP_COMMAND SDCARD_CMD55
#define SDCARD_DISABLE_CRC SDCARD_CMD59
/*----------------------------------------------------------------------------*/
#define GO_IDLE_STATE SDCARD_SOFTWARE_RESET
#define SEND_OP_COND SDCARD_INITIALIZE
#define SEND_IF_COND SDCARD_CMD08
#define SET_BLOCKLEN SDCARD_CHANGE_BLOCK_SIZE
#define READ_SINGLE_BLOCK SDCARD_READ_BLOCK
#define CRC_ON_OFF SDCARD_DISABLE_CRC
/*----------------------------------------------------------------------------*/
#define SDCARD_ARG_NONE 0x00000000
#define SDCARD_ARG_CMD8 0x000001AA
#define SDCARD_CMD00_CRC 0x95
#define SDCARD_CMD01_CRC 0x01
#define SDCARD_CMD08_CRC 0x87
#define SDCARD_CMD55_CRC 0x65
#define SDCARD_DUMMY_DATA 0xFF
#define SDCARD_DUMMY_CRC SDCARD_DUMMY_DATA
/*----------------------------------------------------------------------------*/
#define SDCARD_MMC_MASK 0xC0
#define SDCARD_MMC_CMD_ 0x40
#define SDCARD_SECTOR_SIZE 512
/*----------------------------------------------------------------------------*/
#define SDCARD_RESP_R1_IDLE 0x01
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
void sdcard_flush(int count)
{
	spi_activate(SPI_ACTIVATE);
	while (count>0)
	{
		spi_transfer(SDCARD_DUMMY_DATA);
		count--;
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
unsigned int sdcard_read_block(unsigned int sector, unsigned char* buffer)
{
	int loop;
	unsigned int res, cnt = SDCARD_RESP_WAIT_STEP;
	do
	{
		/* sector size = 512, checksum should already be disabled? */
		res = sdcard_command(READ_SINGLE_BLOCK,sector<<9,SDCARD_DUMMY_CRC);
		sdcard_flush(1);
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
	fb_t* display;
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
	display = video_init(VIDEO_RES_VGA);
	/* blink ERROR_LED indefinitely if failed to init */
	if (!display) error_blink(ERROR_VIDEO_BLINK);
	/* setup screen */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/** say something... */
	video_text_string("--------------------\n");
	video_text_string("SD Card Reader Test!\n");
	video_text_string("--------------------\n\n");
	/** initialize spi */
	video_text_string("Initializing SDCARD... ");
	spi_init(SPI_CLK_DIVIDE_TEST);
	spi_select(SPI_SELECT_1); /* just for fun :p */
	/* initialize sd card */
	sdcard_init();
	video_text_string("done.\n");
	/* software reset */
	video_text_string("Sending idle command... ");
	card = sdcard_command(SDCARD_CMD00,SDCARD_ARG_NONE,SDCARD_CMD00_CRC);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	if (card!=SDCARD_RESP_R1_IDLE) error_blink(ERROR_SDCARD_BLINK);
	sdcard_flush(1);
	/* find out about card? */
	video_text_string("Command CMD8... ");
	card = sdcard_command(SDCARD_CMD08,SDCARD_ARG_CMD8,SDCARD_CMD08_CRC);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	sdcard_flush(1);
	/* old sd card init */
	do
	{
		video_text_string("Command OP COND... ");
		card = sdcard_command(SDCARD_CMD01,SDCARD_ARG_NONE,SDCARD_CMD01_CRC);
		video_text_string("done (0x");
		video_text_hexbyte((unsigned char)card);
		video_text_string(").\n");
		sdcard_flush(1);
	}
	while (card!=0x00);
	/* turn off crc? */
	video_text_string("Command CRC off... ");
	card = sdcard_command(CRC_ON_OFF,SDCARD_ARG_NONE,SDCARD_DUMMY_CRC);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	sdcard_flush(1);
	/* set block length @ sector size */
	video_text_string("Command set block len... ");
	card = sdcard_command(SET_BLOCKLEN,SDCARD_ARG_NONE,SDCARD_DUMMY_CRC);
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	sdcard_flush(1);
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
