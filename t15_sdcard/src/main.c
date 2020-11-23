/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "video.h"
#include "utils.h"
#include "font.h"
#include "spi.h"
#include "sdext.h"
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
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	/** initialize timer */
	timer_init();
	/** initialize video */
	video_init(VIDEO_RES_MAX);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/* say something... */
	video_text_string("--------------------\n");
	video_text_string("SD Card Reader Test!\n");
	video_text_string("--------------------\n\n");
	/** initialize spi */
	spi_init(SPI_CLK_DIVIDE_TEST);
	/* initialize sd card */
	video_text_string("Initializing SDCARD... ");
	sdext_init();
	video_text_string("done.\n");
	/* software reset */
	video_text_string("Sending idle command... ");
	card = sdext_idle();
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	if (card!=SDCARD_RESP_R1_IDLE) error_blink(ERROR_SDCARD_BLINK);
	/* find out about card? */
	video_text_string("Check SD card... ");
	card = sdext_cmd8(sector);
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
			card = sdext_cmd1();
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
			video_text_string("Command ACMD41... "); loop = 0;
			do
			{
				card = sdext_acmd41();
				if (card&SDCARD_ERROR_FLAG)
				{
					video_text_string("failed (0x");
					video_text_hexuint(card);
					video_text_string(").\n");
					if (card&SDCARD_ERROR_CMD55)
					{
						video_text_string("** Cannot send CMD55? Abort.\n");
						error_blink(ERROR_SDCARD_BLINK);
					}
					else if (card&SDCARD_ERROR_ACMD41)
					{
						video_text_string("** Cannot send CMD41? Abort.\n");
						error_blink(ERROR_SDCARD_BLINK);
					}
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
	card = sdext_disable_crc();
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	/* set block length @ sector size */
	video_text_string("Command set block len... ");
	card = sdext_blocksize();
	video_text_string("done (0x");
	video_text_hexbyte((unsigned char)card);
	video_text_string(").\n");
	/* change speed - can get up to 12MHz? */
	/** read a sector... */
	video_text_string("Reading first sector... ");
	card = sdext_read_block(0,sector);
	video_text_string("done. (");
	video_text_hexuint(card);
	video_text_string(")\n\n");
	video_text_string("-- Sector Offset: 0");
	for (loop=0;loop<SDCARD_SECTOR_SIZE;loop++)
	{
		if (!(loop&0xf))
		{
			video_text_string("\n");
			video_text_hexuint(loop);
			video_text_string(":");
		}
		video_text_string(" ");
		video_text_hexbyte(sector[loop]);
	}
	video_text_string("\n");
	/** main loop */
	while(1) { }
}
/*----------------------------------------------------------------------------*/
