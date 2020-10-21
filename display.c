/*
 *	LCD interface example
 *	Uses routines from delay.c
 *	This code will interface to a standard LCD controller
 *	like the Hitachi HD44780. It uses it in 4 bit mode, with
 *	the hardware connected as follows (the standard 14 pin
 *	LCD connector is used):
 *
 *	PORTC bits 0-3 are connected to the LCD data bits 4-7 (high nibble)
 *	PORTC bit 4 is connected to the LCD EN bit (enable)
 *	PORTC bit 5 is connected to the LCD RW input (register write)
 *	PORTC bit 6 is connected to the LCD RS input (register select)
 *
 *	To use these routines, set up the port I/O (TRISC) then
 *	call lcd_init(), then other routines as required.
 *
 */
 
#include <pic18.h>
//#include "delay18.h"
//#include "delays.h"
#include "simdelay.h"
 
#define DelayUs(x) {_delay(x);}
 
//static bit LCD_RS	@ ((unsigned)&PORTA*8+3);	// Register select
//#static bit LCD_EN	@ ((unsigned)&PORTA*8+5);	// Enable
//#static bit LCD_EN	@ ((unsigned)&PORTA*8+5);	// Enable
 
//#define LCD_EN LATCbits.RC4
//#define LCD_RW LATCbits.RC5
//#define LCD_RS LATCbits.RC6
 
volatile static int LCD_RS_flag;
 
//#define	LCD_STROBE	((LCD_EN = 1),(LCD_EN=1),(LCD_EN=0))
//
#define LCD_STROBE() { DelayUs(2); LATC |=  0x10; DelayUs(2); LATC = LATC & 0x0F; DelayUs(2); }
#define LCD_RS(x) {if(x == 1) LATC |=  0x40; else LATC &=  0x0F;}
 
//#define LCD_CHK() {LCD_RW = 1; LCD_RS = 0;  DelayUs(2); LCD_STROBE() ; DelayUs(2); LCD_STROBE();LCD_RW = 0;DelayUs(2)}
 
/* 
 * write a byte to the LCD in 4 bit mode 
 */
void lcd_write(unsigned char c)
{
        LATC = (c >> 4);
        LCD_RS(LCD_RS_flag);
        LCD_STROBE();
        DelayUs(2);
 
        LATC = c & 0xF;
        LCD_RS(LCD_RS_flag);
        LCD_STROBE();
 
        DelayUs(50);
}
 
/*
 * 	Clear and home the LCD
 */
void lcd_clear(void)
{
	LCD_RS_flag = 0;	// write characters
	lcd_write(0x1);
	DelayMs(2);
}
 
/* 
 * write a byte to the LCD in 4 bit mode 
 */
#define lcd_putc(x) lcd_putchar(x)
#define lcd_putch(x) lcd_putchar(x)
 
void lcd_putchar(char s){
	LCD_RS_flag = 1;	// write characters
	lcd_write(s);
}
 
/* 
 * write a string of chars to the LCD 
 */ 
void lcd_puts(const char * s)
{
	LCD_RS_flag = 1;	// write characters
	while(*s){
		lcd_write(*s++);
 
        }
}
 
/*
void lcd_putsr(const rom char * s)
{
	LCD_RS_flag = 1;	// write characters
	while(*s){
		lcd_write(*s++);
        }
}
 */
 
/*
 * write byte (hex)
 */
void lcd_puthex(unsigned char i)
{
        unsigned char c;
	LCD_RS_flag = 1;	// write characters
 
        c = i >> 4;
        if (c>9) {
                lcd_write('A'-10+c);
        } else {
                lcd_write('0'+c);
        }
 
        c = i & 0x0F;
        if (c>9) {
                lcd_write('A'-10+c);
        } else {
                lcd_write('0'+c);
        }
 
 
}
 
/*
 * Go to the specified position
 */
void
lcd_goto(unsigned char pos)
{
	LCD_RS_flag = 0;
	lcd_write(0x80+pos);
}
 
/** 
 * initialise the LCD - put into 4 bit mode 
 *      4 bit mode, 2 lines
 */
void lcd_init(void)
{
	LCD_RS(0);	// write control bytes
    LCD_RS_flag = 0;
	DelayMs(60);	// power on delay
 
	LATC = 0x03;	// FN set 1
	LCD_STROBE();
	DelayMs(10);     
 
	LCD_STROBE();     // fn set 2
    DelayUs(50);
 
	LCD_STROBE();     // FN set 3
	DelayUs(50);
 
	LATC = 0x2;	// FN set #4 set 4 bit mode
	LCD_STROBE();
    DelayUs(50);
 
	lcd_write(0x29);	// 4 bit mode, 
 
	lcd_write(0x1D);	// int osc freq
    lcd_write(0x79);	// contrast set
    lcd_write(0x50);	// power / icon / contrast ctrl
    lcd_write(0x6C);	// follower control
 
	lcd_write(0x28);	// 4 bit mode, 1/16 duty, 5x8 font, radkovani
	lcd_write(0x0E);	// display on, blink curson on
	lcd_write(0x01);	// entry mode set
	lcd_write(0x06);	// entry mode set
    return;
}
 
void lcd_init33(void)
{
    LCD_RS_flag = 0;
 	LCD_RS(0);	// write control bytes
 
	DelayMs(60);	// power on delay
 
	LATC = 0x03;	// FN set 1
	LCD_STROBE();
	DelayMs(10);
 
	LCD_STROBE();     // fn set 2
    DelayUs(50);
 
	LCD_STROBE();     // FN set 3
	DelayUs(50);
 
	LATC = 0x2;	// FN set #4 set 4 bit mode
	LCD_STROBE();
    DelayUs(50);
 
	lcd_write(0x29);	// 4 bit mode,
 
	//lcd_write(0x1D);	// int osc freq
	lcd_write(0x1F);	// int osc freq
 
    lcd_write(0x79);	// contrast set
	lcd_write(0x50 + 0x04);	// power / icon / contrast ctrl
    lcd_write(0x6E);	// follower control
 
	lcd_write(0x28);	// 4 bit mode, 1/16 duty, 5x8 font, radkovani
	lcd_write(0x0f);	// display on, blink curson on
	lcd_write(0x01);	// clear
	lcd_write(0x06);	// cursor autoincrement
    return;
}