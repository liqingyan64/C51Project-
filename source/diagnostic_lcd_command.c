#include <REG51.H>

typedef unsigned char uchar;
typedef unsigned int uint;

#define LCD_DATA P0

sbit LCD_A0  = P2^0;
sbit LCD_RW  = P2^1;
sbit LCD_E   = P2^2;
sbit LCD_CS  = P2^3;
sbit LCD_RES = P2^4;

sbit GREEN_LED  = P3^0;
sbit RED_LED    = P3^2;
sbit YELLOW_LED = P3^3;

void delay_us(uint n)
{
    while (n--);
}

void delay_ms(uint ms)
{
    uint i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void lcd_cmd_cs_low(uchar cmd)
{
    LCD_CS = 0;
    LCD_A0 = 0;
    LCD_RW = 0;
    LCD_DATA = cmd;
    delay_us(20);
    LCD_E = 1;
    delay_us(80);
    LCD_E = 0;
    delay_us(80);
    LCD_CS = 1;
}

void lcd_cmd_cs_high(uchar cmd)
{
    LCD_CS = 1;
    LCD_A0 = 0;
    LCD_RW = 0;
    LCD_DATA = cmd;
    delay_us(20);
    LCD_E = 1;
    delay_us(80);
    LCD_E = 0;
    delay_us(80);
    LCD_CS = 0;
}

void init_by_cs_low(void)
{
    LCD_CS = 1;
    LCD_E = 0;
    LCD_RES = 0;
    delay_ms(100);
    LCD_RES = 1;
    delay_ms(100);

    lcd_cmd_cs_low(0xE2);
    delay_ms(20);
    lcd_cmd_cs_low(0xA0);
    lcd_cmd_cs_low(0xC8);
    lcd_cmd_cs_low(0xA2);
    lcd_cmd_cs_low(0x2F);
    lcd_cmd_cs_low(0x81);
    lcd_cmd_cs_low(0x3F);
    lcd_cmd_cs_low(0xAF);
    lcd_cmd_cs_low(0xA5); /* all display points ON */
}

void init_by_cs_high(void)
{
    LCD_CS = 0;
    LCD_E = 0;
    LCD_RES = 0;
    delay_ms(100);
    LCD_RES = 1;
    delay_ms(100);

    lcd_cmd_cs_high(0xE2);
    delay_ms(20);
    lcd_cmd_cs_high(0xA0);
    lcd_cmd_cs_high(0xC8);
    lcd_cmd_cs_high(0xA2);
    lcd_cmd_cs_high(0x2F);
    lcd_cmd_cs_high(0x81);
    lcd_cmd_cs_high(0x3F);
    lcd_cmd_cs_high(0xAF);
    lcd_cmd_cs_high(0xA5); /* all display points ON */
}

void main(void)
{
    P0 = 0xFF;
    P2 = 0xFF;
    P3 = 0x00;

    while (1) {
        GREEN_LED = 1;
        RED_LED = 0;
        YELLOW_LED = 0;
        init_by_cs_low();
        delay_ms(3000);

        GREEN_LED = 0;
        RED_LED = 1;
        YELLOW_LED = 0;
        init_by_cs_high();
        delay_ms(3000);

        GREEN_LED = 0;
        RED_LED = 0;
        YELLOW_LED = 1;
        delay_ms(1000);
    }
}
