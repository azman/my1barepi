/*----------------------------------------------------------------------------*/
#include "gpio.h"
#include "timer.h"
#include "mailbox.h"
#include "video.h"
#include "utils.h"
#include "font.h"
/*----------------------------------------------------------------------------*/
#define ERROR_LED 47
/*----------------------------------------------------------------------------*/
void blink_error(int count)
{
	while(count>0)
	{
		gpio_toggle(ERROR_LED); timer_wait(TIMER_S/2);
		gpio_toggle(ERROR_LED); timer_wait(TIMER_S/2);
		count--;
	}
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	char buff[16];
	fb_t* fbuff;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	timer_init();
	/** initialize video stuff - mailbox before that! */
	mailbox_init();
	do
	{
		blink_error(2);
		timer_wait(TIMER_S);
		fbuff = video_init();
	}
	while (!fbuff);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	/** write something */
	video_text_char('H');
	video_text_char('E');
	video_text_char('L');
	video_text_char('L');
	video_text_char('O');
	video_text_char('!');
	video_text_string("\nHAHA!");
	video_text_cursor(3,1);
	video_text_string("Welcome!");
	/* display info */
	video_text_newline();
	video_text_string("DISPX: ");
	int2str(buff,fbuff->screen.xres);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("DISPY: ");
	int2str(buff,fbuff->screen.yres);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("VDISPX: ");
	int2str(buff,fbuff->screen.xout);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("VDISPY: ");
	int2str(buff,fbuff->screen.yout);
	video_text_string(buff);
	video_text_char('\n');
	video_text_string("OFFX: ");
	int2str(buff,fbuff->screen.xoff);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("OFFY: ");
	int2str(buff,fbuff->screen.yoff);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("XMAX: ");
	int2str(buff,fbuff->cursor.xmax);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("YMAX: ");
	int2str(buff,fbuff->cursor.ymax);
	video_text_string(buff);
	video_text_char('\n');
	video_text_string("PITCH: ");
	int2str(buff,fbuff->screen.pskip);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("DEPTH: ");
	int2str(buff,fbuff->screen.depth);
	video_text_string(buff);
	/* test col overflow */
	video_text_cursor(fbuff->cursor.y+1,fbuff->cursor.xmax-5);
	video_text_string("Continue next line!");
	/* test line drawing */
	video_draw_line(100,100,200,100);
	video_draw_line(200,100,100,200);
	video_draw_line(100,200,100,100);
	video_text_string("\nWait for it...");
	blink_error(3);
	/* test row overflow */
	video_text_cursor(fbuff->cursor.ymax-1,fbuff->cursor.xmax-5);
	video_text_string("Scroll this!");
	/** sticking around */
	while(1);
}
/*----------------------------------------------------------------------------*/
