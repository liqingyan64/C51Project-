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

void pulse_e_normal(void)
{
    LCD_E = 1;
    delay_us(100);
    LCD_E = 0;
    delay_us(100);
}

void pulse_e_invert(void)
{
    LCD_E = 0;
    delay_us(100);
    LCD_E = 1;
    delay_us(100);
}

void write_bus(uchar value, bit is_data, bit cs_high, bit a0_invert, bit e_invert)
{
    LCD_DATA = value;
    LCD_RW = 0;
    LCD_A0 = a0_invert ? !is_data : is_data;
    LCD_CS = cs_high ? 1 : 0;
    delay_us(50);

    if (e_invert) {
        pulse_e_invert();
    } else {
        pulse_e_normal();
    }

    LCD_CS = cs_high ? 0 : 1;
    delay_us(100);
}

void send_cmd(uchar cmd, bit cs_high, bit a0_invert, bit e_invert)
{
    write_bus(cmd, 0, cs_high, a0_invert, e_invert);
}

void send_data(uchar dat, bit cs_high, bit a0_invert, bit e_invert)
{
    write_bus(dat, 1, cs_high, a0_invert, e_invert);
}

void init_lcd(bit cs_high, bit a0_invert, bit e_invert)
{
    LCD_DATA = 0xFF;
    LCD_CS = cs_high ? 0 : 1;
    LCD_E = e_invert ? 1 : 0;
    LCD_RW = 0;

    LCD_RES = 0;
    delay_ms(100);
    LCD_RES = 1;
    delay_ms(100);

    send_cmd(0xE2, cs_high, a0_invert, e_invert);
    delay_ms(20);
    send_cmd(0xA0, cs_high, a0_invert, e_invert);
    send_cmd(0xC8, cs_high, a0_invert, e_invert);
    send_cmd(0xA2, cs_high, a0_invert, e_invert);
    send_cmd(0x2F, cs_high, a0_invert, e_invert);
    send_cmd(0x81, cs_high, a0_invert, e_invert);
    send_cmd(0x3F, cs_high, a0_invert, e_invert);
    send_cmd(0xAF, cs_high, a0_invert, e_invert);
}

void set_pos(uchar page, uchar col, bit cs_high, bit a0_invert, bit e_invert)
{
    send_cmd(0xB0 | page, cs_high, a0_invert, e_invert);
    send_cmd(0x10 | (col >> 4), cs_high, a0_invert, e_invert);
    send_cmd(0x00 | (col & 0x0F), cs_high, a0_invert, e_invert);
}

void fill_screen(uchar pattern, bit cs_high, bit a0_invert, bit e_invert)
{
    uchar page, col;
    for (page = 0; page < 8; page++) {
        set_pos(page, 0, cs_high, a0_invert, e_invert);
        for (col = 0; col < 128; col++) {
            send_data(pattern, cs_high, a0_invert, e_invert);
        }
    }
}

void test_mode(bit cs_high, bit a0_invert, bit e_invert)
{
    init_lcd(cs_high, a0_invert, e_invert);
    fill_screen(0xFF, cs_high, a0_invert, e_invert);
    delay_ms(1500);
    fill_screen(0x00, cs_high, a0_invert, e_invert);
    delay_ms(800);
    fill_screen(0xAA, cs_high, a0_invert, e_invert);
    delay_ms(1500);
}

void main(void)
{
    P0 = 0xFF;
    P2 = 0xFF;
    P3 = 0x00;

    while (1) {
        GREEN_LED = 1; RED_LED = 0; YELLOW_LED = 0;
        test_mode(0, 0, 0);

        GREEN_LED = 0; RED_LED = 1; YELLOW_LED = 0;
        test_mode(1, 0, 0);

        GREEN_LED = 0; RED_LED = 0; YELLOW_LED = 1;
        test_mode(0, 1, 0);

        GREEN_LED = 1; RED_LED = 1; YELLOW_LED = 0;
        test_mode(1, 1, 0);

        GREEN_LED = 1; RED_LED = 0; YELLOW_LED = 1;
        test_mode(0, 0, 1);

        GREEN_LED = 0; RED_LED = 1; YELLOW_LED = 1;
        test_mode(1, 0, 1);
    }
}
