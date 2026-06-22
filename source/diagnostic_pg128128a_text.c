#include <REG51.H>

typedef unsigned char uchar;
typedef unsigned int uint;

#define LCD_DATA P0

sbit LCD_CD  = P2^0;
sbit LCD_RD  = P2^1;
sbit LCD_WR  = P2^2;
sbit LCD_CE  = P2^3;
sbit LCD_RST = P2^4;

sbit GREEN_LED  = P3^0;
sbit RED_LED    = P3^2;
sbit YELLOW_LED = P3^3;

#define LCD_COLS 16
#define TEXT_HOME 0x0000
#define GRAPH_HOME 0x0200

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

void lcd_wait(void)
{
    uint timeout = 60000;
    uchar status;

    LCD_DATA = 0xFF;
    LCD_CD = 1;
    LCD_WR = 1;
    LCD_CE = 0;

    do {
        LCD_RD = 0;
        delay_us(5);
        status = LCD_DATA;
        LCD_RD = 1;
        if ((status & 0x03) == 0x03) {
            break;
        }
    } while (--timeout);

    LCD_CE = 1;
}

void lcd_data(uchar dat)
{
    lcd_wait();
    LCD_DATA = dat;
    LCD_CD = 0;
    LCD_RD = 1;
    LCD_WR = 0;
    LCD_CE = 0;
    delay_us(10);
    LCD_CE = 1;
    LCD_WR = 1;
}

void lcd_cmd(uchar cmd)
{
    lcd_wait();
    LCD_DATA = cmd;
    LCD_CD = 1;
    LCD_RD = 1;
    LCD_WR = 0;
    LCD_CE = 0;
    delay_us(10);
    LCD_CE = 1;
    LCD_WR = 1;
}

void lcd_2data_cmd(uint value, uchar cmd)
{
    lcd_data((uchar)(value & 0xFF));
    lcd_data((uchar)(value >> 8));
    lcd_cmd(cmd);
}

void lcd_set_addr(uint addr)
{
    lcd_2data_cmd(addr, 0x24);
}

void lcd_clear(void)
{
    uint i;

    lcd_set_addr(TEXT_HOME);
    for (i = 0; i < 256; i++) {
        lcd_data(0x00);
        lcd_cmd(0xC0);
    }

    lcd_set_addr(GRAPH_HOME);
    for (i = 0; i < 2048; i++) {
        lcd_data(0x00);
        lcd_cmd(0xC0);
    }
}

void lcd_init(void)
{
    LCD_DATA = 0xFF;
    LCD_CE = 1;
    LCD_RD = 1;
    LCD_WR = 1;
    LCD_CD = 1;

    LCD_RST = 0;
    delay_ms(100);
    LCD_RST = 1;
    delay_ms(100);

    lcd_2data_cmd(TEXT_HOME, 0x40);
    lcd_2data_cmd(LCD_COLS, 0x41);
    lcd_2data_cmd(GRAPH_HOME, 0x42);
    lcd_2data_cmd(LCD_COLS, 0x43);
    lcd_cmd(0x80);
    lcd_cmd(0x94);
    lcd_clear();
}

void lcd_put_char(uchar row, uchar col, char ch)
{
    if (ch < 0x20 || ch > 0x7F) {
        ch = ' ';
    }
    lcd_set_addr(TEXT_HOME + row * LCD_COLS + col);
    lcd_data(ch - 0x20);
    lcd_cmd(0xC0);
}

void lcd_puts(uchar row, uchar col, char *s)
{
    while (*s && col < LCD_COLS) {
        lcd_put_char(row, col, *s);
        col++;
        s++;
    }
}

void main(void)
{
    P0 = 0xFF;
    P2 = 0xFF;
    P3 = 0x00;

    GREEN_LED = 1;
    RED_LED = 0;
    YELLOW_LED = 0;

    lcd_init();
    lcd_puts(0, 0, "PG128128A OK");
    lcd_puts(2, 0, "PASSWORD LOCK");
    lcd_puts(4, 0, "P0 DATA BUS");
    lcd_puts(6, 0, "P2 CONTROL");

    while (1) {
        GREEN_LED = 1;
        RED_LED = 0;
        YELLOW_LED = 0;
        delay_ms(500);
        GREEN_LED = 0;
        RED_LED = 1;
        YELLOW_LED = 0;
        delay_ms(500);
    }
}
