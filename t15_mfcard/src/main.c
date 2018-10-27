/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"
#include "utils.h"
#include "font.h"
#include "spi.h"
#include "frc522.h"
/*----------------------------------------------------------------------------*/
#define ERROR_LED 47
#define SPI_CLK_DIVIDE_TEST 26
#define MIFARE_UID_SIZE 5
/*----------------------------------------------------------------------------*/
void main(void)
{
	fb_t* display;
	int loop;
	unsigned char save[MIFARE_UID_SIZE];
	unsigned char data[MAX_SIZE];
	unsigned int last = 0;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	/** initialize timer */
	timer_init();
	/** initialize spi */
	spi_init(SPI_CLK_DIVIDE_TEST);
	spi_select(SPI_SELECT_1); /* just for fun :p */
	/* initialize mf contactless card reader */
	frc522_init();
	/** initialize mailbox */
	mailbox_init();
	/** initialize video */
	display = video_init(VIDEO_RES_VGA);
	/* blink ERROR_LED indefinitely if failed to init */
	if (!display)
	{
		while(1)
		{
			gpio_toggle(ERROR_LED);
			timer_wait(TIMER_S/2);
		}
	}
	/* setup screen */
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
		while(1)
		{
			gpio_toggle(ERROR_LED);
			timer_wait(TIMER_S);
		}
	}
	video_text_string("FRC522 found. Firmware version is 0x");
	video_text_hexuint(loop);
	video_text_string(".\n");
	loop = frc522_digital_self_test();
	if (!loop)
		video_text_string("Digital self test passed.\n");
	else if (loop<0)
		video_text_string("Unknown firmware version.\n");
	else
	{
		video_text_string("Digital self test failed @");
		video_text_integer(loop);
		video_text_string(".\n");
	}
	/** main loop */
	while(1)
	{
		if (frc522_reqtag(MF1_REQIDL,data)==FRC522_OK)
		{
			if (frc522_anti_collision(data)==FRC522_OK)
			{
				for (loop=0;loop<MIFARE_UID_SIZE;loop++)
					if (data[loop]!=save[loop]) break;
				if (loop==MIFARE_UID_SIZE)
					if ((timer_read()-last)<2*TIMER_S)
						continue;
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
}
/*----------------------------------------------------------------------------*/
