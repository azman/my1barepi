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
	screen_t *pscr;
	cursor_t *pcur;
	fbinfo_t* pinfo;
	int check;
	/** initialize gpio */
	gpio_init();
	gpio_config(ERROR_LED,GPIO_OUTPUT);
	timer_init();
	/** initialize video stuff - mailbox before that! */
	mailbox_init();
	check = video_init(0x0);
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	blink_error(check);
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
	pscr = video_get_screen();
	pcur = video_get_cursor();
	pinfo = video_get_fbinfo();
	video_text_string("YMAX: ");
	int2str(buff,pcur->ymax);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("XMAX: ");
	int2str(buff,pcur->xmax);
	video_text_string(buff);
	video_text_char('\n');
	video_text_string("DISPY: ");
	int2str(buff,pinfo->height);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("DISPX: ");
	int2str(buff,pinfo->width);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("VDISPY: ");
	int2str(buff,pinfo->vheight);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("VDISPX: ");
	int2str(buff,pinfo->vwidth);
	video_text_string(buff);
	video_text_char('\n');
	video_text_string("OFFY: ");
	int2str(buff,pinfo->yoffset);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("OFFX: ");
	int2str(buff,pinfo->xoffset);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("DEPTH: ");
	int2str(buff,pinfo->depth);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("PITCH: ");
	int2str(buff,pinfo->pitch);
	video_text_string(buff);
	video_text_char('\n');
	video_text_string("SCRY: ");
	int2str(buff,pscr->yres);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("SCRX: ");
	int2str(buff,pscr->xres);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("SIZE: ");
	int2str(buff,pscr->psize);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("SKIP: ");
	int2str(buff,pscr->pskip);
	video_text_string(buff);
	video_text_string(", ");
	video_text_string("BPP: ");
	int2str(buff,pscr->bpp);
	video_text_string(buff);
	/* test col overflow */
	video_text_cursor(pcur->y+1,pcur->xmax-5);
	video_text_string("Continue next line!");
	/* test line drawing */
	video_draw_line(100,100,200,100);
	video_draw_line(200,100,100,200);
	video_draw_line(100,200,100,100);
	video_text_string("\nWait for it...");
	blink_error(3);
	/* test row overflow */
	video_text_cursor(pcur->ymax-1,pcur->xmax-5);
	video_text_string("Scroll this!");
	/** sticking around */
	while(1);
}
/*----------------------------------------------------------------------------*/
