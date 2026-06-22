#include <REG51.H>

typedef unsigned char uchar;
typedef unsigned int uint;

sbit GREEN_LED  = P3^0;
sbit RED_LED    = P3^2;
sbit YELLOW_LED = P3^3;

char code key_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

void delay_ms(uint ms)
{
    uint i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

char key_scan(void)
{
    uchar row;
    uchar col_value;

    for (row = 0; row < 4; row++) {
        P1 = 0xFF;
        P1 &= ~(1 << row);
        delay_ms(2);
        col_value = P1 & 0xF0;

        if (col_value != 0xF0) {
            delay_ms(15);
            col_value = P1 & 0xF0;
            if (col_value != 0xF0) {
                if ((col_value & 0x10) == 0) {
                    while ((P1 & 0x10) == 0);
                    return key_map[row][0];
                }
                if ((col_value & 0x20) == 0) {
                    while ((P1 & 0x20) == 0);
                    return key_map[row][1];
                }
                if ((col_value & 0x40) == 0) {
                    while ((P1 & 0x40) == 0);
                    return key_map[row][2];
                }
                if ((col_value & 0x80) == 0) {
                    while ((P1 & 0x80) == 0);
                    return key_map[row][3];
                }
            }
        }
    }
    return 0;
}

void show_key_on_led(char key)
{
    GREEN_LED = 0;
    RED_LED = 0;
    YELLOW_LED = 0;

    if (key == '1' || key == '4' || key == '7' || key == '*') {
        GREEN_LED = 1;
    } else if (key == '2' || key == '5' || key == '8' || key == '0') {
        RED_LED = 1;
    } else if (key == '3' || key == '6' || key == '9' || key == '#') {
        YELLOW_LED = 1;
    } else if (key == 'A' || key == 'B' || key == 'C' || key == 'D') {
        GREEN_LED = 1;
        RED_LED = 1;
        YELLOW_LED = 1;
    }
}

void main(void)
{
    char key;
    P1 = 0xFF;
    P3 = 0x00;

    while (1) {
        key = key_scan();
        if (key != 0) {
            show_key_on_led(key);
            delay_ms(300);
            GREEN_LED = 0;
            RED_LED = 0;
            YELLOW_LED = 0;
        }
    }
}
