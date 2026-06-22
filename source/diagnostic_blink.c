#include <REG51.H>

sbit GREEN_LED  = P3^0;
sbit BUZZER     = P3^1;
sbit RED_LED    = P3^2;
sbit YELLOW_LED = P3^3;

void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 120; j++);
    }
}

void main(void)
{
    P3 = 0x00;

    while (1) {
        GREEN_LED = 1;
        RED_LED = 0;
        YELLOW_LED = 0;
        BUZZER = 1;
        delay_ms(300);

        GREEN_LED = 0;
        RED_LED = 1;
        YELLOW_LED = 0;
        BUZZER = 0;
        delay_ms(300);

        GREEN_LED = 0;
        RED_LED = 0;
        YELLOW_LED = 1;
        delay_ms(300);
    }
}
