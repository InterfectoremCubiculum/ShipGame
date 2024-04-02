#include "stdio.h"
#define _TURN_ON					0x0C	// Turn Lcd display on
#define _TURN_OFF					0x08	// Turn Lcd display off
#define _FIRST_ROW					0x80	// Move cursor to the 1st row
#define _SECOND_ROW					0xC0	// Move cursor to the 2nd row
#define _CLEAR						0x01	// Clear display
#define _RETURN_HOME				0x02	// Return cursor to home position, returns a shifted display to its original position. Display data RAM is unaffected.
#define _CURSOR_OFF					0x0C	// Turn off cursor
#define _UNDERLINE_ON				0x0E	// Underline cursor on
#define _BLINK_CURSOR_ON		    0x0F	// Blink cursor on
#define _MOVE_CURSOR_LEFT		    0x10	// Move cursor left without changing display data RAM
#define _MOVE_CURSOR_RIGHT	        0x14	// Move cursor right without changing display data RAM
#define _SHIFT_LEFT					0x18	// Shift display left without changing display data RAM
#define _SHIFT_RIGHT				0x1C	// Shift display right without changing display data RAM
#define _LCD_4BIT					0x00
#define _LCD_8BIT					0x10
#define _LCD_FONT_5x8				0x00
#define _LCD_FONT_5x10			    0x04
#define _LCD_1LINE					0x00
#define _LCD_2LINE					0x08

#define _LCD_INIT					0x06

//#define _LCD_INIT					0x04
//#define _LCD_INIT					0x05
//#define _LCD_INIT					0x07

void lcd_delay(void);
void lcd_cmd(char out_char);
void lcd_char_cp(char out_char);
void lcd_char(unsigned char row, unsigned char column, char out_char);
void lcd_out_cp(char *out_char);
void lcd_print(unsigned char row, unsigned char column, char *out_char);
void lcd_gotoxy(unsigned char row, unsigned char column);
void lcd_init(char bits, char font, char lines);
void lcd_clear(void);
void lcd_line1(void);
void lcd_line2(void);
