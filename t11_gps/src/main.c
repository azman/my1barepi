/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "uart.h"
#include "string.h"
#include "mailbox.h"
#include "video.h"
/*----------------------------------------------------------------------------*/
#define BUFF_SIZE 128
/*----------------------------------------------------------------------------*/
void main(void)
{
	int size = 0, test;
	char buff[BUFF_SIZE];
	/** initialize stuffs */
	gpio_init();
	timer_init();
	/** initialize uart with default baudrate */
	uart_init(UART_BAUD_DEFAULT);
	/** initialize mailbox */
	mailbox_init();
	video_init(VIDEO_RES_VGA);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/** send out the word! */
	video_text_string("\n\nYAY! It is alive!\n");
	video_text_string("\nWaiting... ");
	/** do the thing... */
	while(1)
	{
		if (uart_incoming())
		{
			test = uart_read();
			if (size<BUFF_SIZE-1)
			{
				buff[size++] = (unsigned char) test;
				if (size>1&&test=='\n')
				{
					buff[size] = 0x0;
					size = 0;
					/** process */
					if (!strncmp(buff,"$GPGLL",6))
					{
						video_text_string("Found: ");
						video_text_string(buff);
						video_text_string("\nWaiting...");
					}
					else
					{
						video_text_string("What?: ");
						video_text_string(buff);
						video_text_string("\nWaiting...");
					}
				}
			}
			else
			{
				buff[size] = 0x0;
				size = 0;
				video_text_string("Overflow: ");
				video_text_string(buff);
				video_text_string("\nWaiting...");
			}
		}
	}
}
/*----------------------------------------------------------------------------*/
