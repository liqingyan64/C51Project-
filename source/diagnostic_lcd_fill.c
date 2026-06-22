#include <REG51.H>

typedef unsigned char uchar;
typedef unsigned int uint;

#define LCD_DATA P0

sbit LCD_A0  = P2^0;
sbit LCD_RW  = P2^1;
sbit LCD_E   = P2^2;
sbit LCD_CS  = P2^3;
sbit LCD_RES = P2^4;

sbit BUZZER = P3^1;

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

void buzzer_sound(uint ms)
{
    uint i;
    for (i = 0; i < ms; i++) {
        BUZZER = 1;
        delay_us(80);
        BUZZER = 0;
        delay_us(80);
    }
}

void lcd_write(uchar is_data, uchar value)
{
    LCD_CS = 0;
    LCD_A0 = is_data;
    LCD_RW = 0;
    LCD_DATA = value;
    delay_us(10);
    LCD_E = 1;
    delay_us(40);
    LCD_E = 0;
    delay_us(40);
    LCD_CS = 1;
}

void lcd_cmd(uchar cmd)
{
    lcd_write(0, cmd);
}

void lcd_data(uchar dat)
{
    lcd_write(1, dat);
}

void lcd_set_pos(uchar page, uchar col)
{
    lcd_cmd(0xB0 | (page & 0x0F));
    lcd_cmd(0x10 | ((col >> 4) & 0x0F));
    lcd_cmd(0x00 | (col & 0x0F));
}

void lcd_fill(uchar pattern)
{
    uchar page, col;
    for (page = 0; page < 8; page++) {
        lcd_set_pos(page, 0);
        for (col = 0; col < 128; col++) {
            lcd_data(pattern);
        }
    }
}

void lcd_init(void)
{
    P0 = 0xFF;
    P2 = 0xFF;

    LCD_E = 0;
    LCD_CS = 1;
    LCD_RW = 0;
    LCD_A0 = 0;

    LCD_RES = 0;
    delay_ms(100);
    LCD_RES = 1;
    delay_ms(100);

    lcd_cmd(0xE2);
    delay_ms(20);
    lcd_cmd(0xA0);
    lcd_cmd(0xC8);
    lcd_cmd(0xA2);
    lcd_cmd(0x2F);
    delay_ms(20);
    lcd_cmd(0x27);
    lcd_cmd(0x81);
    lcd_cmd(0x3F);
    lcd_cmd(0x40);
    lcd_cmd(0xAF);
}

void main(void)
{
    lcd_init();

    while (1) {
        buzzer_sound(200);
        lcd_fill(0xFF);
        delay_ms(1000);
        lcd_fill(0x00);
        delay_ms(1000);
        lcd_fill(0xAA);
        delay_ms(1000);
        lcd_fill(0x55);
        delay_ms(1000);
    }
}
