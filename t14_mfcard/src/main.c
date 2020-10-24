/*----------------------------------------------------------------------------*/
#include "timer.h"
#include "video.h"
#include "utils.h"
#include "spi.h"
#include "frc522.h"
/*----------------------------------------------------------------------------*/
#define SPI_CLK_DIVIDE_TEST 26
#define MIFARE_UID_SIZE 5
/*----------------------------------------------------------------------------*/
void main(void)
{
	int loop;
	unsigned char save[MIFARE_UID_SIZE];
	unsigned char data[MAX_SIZE];
	unsigned int last = 0;
	/** initialize timer */
	timer_init();
	/** initialize spi */
	spi_init(SPI_CLK_DIVIDE_TEST);
	spi_select(SPI_SELECT_1); /* just for fun :p */
	/* initialize mf contactless card reader */
	frc522_init();
	/** initialize video */
	video_init(VIDEO_RES_VGA);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/** say something... */
	video_text_string("--------------------\n");
	video_text_string("MF Card Reader Test!\n");
	video_text_string("--------------------\n\n");
	loop = frc522_get_firmware_version();
	if (!loop)
	{
		video_text_string("Cannot find FRC522 hardware! Aborting!\n");
		while(1);
	}
	video_text_string("FRC522 found. Firmware version is 0x");
	video_text_hexuint(loop);
	video_text_string(".\n-- ");
	loop = frc522_digital_self_test();
	if (!loop)
		video_text_string("Digital self test passed.\n");
	else if (loop<0)
		video_text_string("Unknown firmware version "
			"(But may still work!).\n");
	else
	{
		video_text_string("Digital self test failed @");
		video_text_integer(loop);
		video_text_string(".\n");
	}
	/** main loop */
	while(1)
	{
		if (frc522_get_card_id(data)==FRC522_OK)
		{
			/* check if we got the same id again... */
			for (loop=0;loop<MIFARE_UID_SIZE;loop++)
				if (data[loop]!=save[loop]) break;
			if (loop==MIFARE_UID_SIZE)
				if ((timer_read()-last)<2*TIMER_S)
					continue; /* skip if within lapse */
			/* show id! */
			video_text_string("ID: ");
			for (loop=0;loop<MIFARE_UID_SIZE;loop++)
				video_text_hexbyte(data[loop]);
			video_text_string("\n");
			for (loop=0;loop<MIFARE_UID_SIZE;loop++)
				save[loop]=data[loop];
			last = timer_read();
		}
	}
}
/*----------------------------------------------------------------------------*/
