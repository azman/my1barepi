/*----------------------------------------------------------------------------*/
#ifndef __MY1TEXTLCD_H
#define __MY1TEXTLCD_H
/*----------------------------------------------------------------------------*/
/**
 * LCD Interface Library
 *   - for text lcd using HD44780U controller/driver
 *   - 16-pin interface:
 *     = gnd(gnd),vcc(5v),vl(n/c),rs(18),rw(19),e(17)
 *     = d0-d7(20-27),ba(n/c),bk(n/c)
**/
/*----------------------------------------------------------------------------*/
/** LCD_DATA: GPIO27-GPIO20 MSB is also the busy status line */
#define LCD_ENB_GPIO 17
#define LCD_DNC_GPIO 18
#define LCD_RNW_GPIO 19
#define LCD_BUSY_GPIO 27
/*----------------------------------------------------------------------------*/
#define LCD_PULSE_E TIMER_US
#define LCD_DELAY_E 25*TIMER_US
/*----------------------------------------------------------------------------*/
/** lcd init reset sequence */
#define LCD_CMD_INITSEQ 0x30
/** 8-bit, 2-lines, 5x8 dots font */
#define LCD_CMD_CONF_B8L2D5x7 0x38
/** display off */
#define LCD_CMD_CONF_DISP0 0x08
/** display on, cursor off, blinking off */
#define LCD_CMD_CONF_D1C0B0 0x0C
/** addr increment, shift diplay off - entry mode set? */
#define LCD_CMD_CONF_CIS0 0x06
/** clear diplay, ram addr 0 */
#define LCD_CMD_CLEAR 0x01
/** lcd max char per row */
#define LCD_MAX_CHAR 16
/** lcd line cursor positions */
#define LCD_POS_LINE1 0x80
#define LCD_POS_LINE2 0xC0
/*----------------------------------------------------------------------------*/
typedef unsigned char lcdbyte;
/*----------------------------------------------------------------------------*/
void lcd_init(void);
void lcd_send_command(lcdbyte command);
void lcd_send_data(lcdbyte asciidat);
void lcd_print(char* message);
/*----------------------------------------------------------------------------*/
/** useful macro */
#define lcd_goto_line1() lcd_send_command(LCD_POS_LINE1)
#define lcd_goto_line2() lcd_send_command(LCD_POS_LINE2)
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
