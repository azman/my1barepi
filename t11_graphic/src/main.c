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
	char temp[16];
	fb_t* display;
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
		display = video_init(VIDEO_RES_VGA);
	}
	while (!display);
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
	int2str(temp,display->screen.xres);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("DISPY: ");
	int2str(temp,display->screen.yres);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("VDISPX: ");
	int2str(temp,display->screen.xout);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("VDISPY: ");
	int2str(temp,display->screen.yout);
	video_text_string(temp);
	video_text_char('\n');
	video_text_string("OFFX: ");
	int2str(temp,display->screen.xoff);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("OFFY: ");
	int2str(temp,display->screen.yoff);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("XMAX: ");
	int2str(temp,display->cursor.xmax);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("YMAX: ");
	int2str(temp,display->cursor.ymax);
	video_text_string(temp);
	video_text_char('\n');
	video_text_string("PITCH: ");
	int2str(temp,display->screen.pskip);
	video_text_string(temp);
	video_text_string(", ");
	video_text_string("DEPTH: ");
	int2str(temp,display->screen.depth);
	video_text_string(temp);
	/* test col overflow */
	video_text_cursor(display->cursor.y+1,display->cursor.xmax-5);
	video_text_string("Continue next line!");
	/* test line drawing */
	video_draw_line(100,100,200,100);
	video_draw_line(200,100,100,200);
	video_draw_line(100,200,100,100);
	video_text_string("\nWait for it...");
	blink_error(3);
	/* test row overflow */
	video_text_cursor(display->cursor.ymax-1,display->cursor.xmax-5);
	video_text_string("Scroll this!");
	/** sticking around */
	while(1);
}
/*----------------------------------------------------------------------------*/
