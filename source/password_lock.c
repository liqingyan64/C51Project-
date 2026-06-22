#include <REG51.H>

typedef unsigned char uchar;
typedef unsigned int uint;

#include "unlock_animation.h"

#define LCD_DATA P0

/* PG128128A / T6963C connections */
sbit LCD_CD  = P2^0;  /* C/D: 1 command/status, 0 data */
sbit LCD_RD  = P2^1;  /* active low */
sbit LCD_WR  = P2^2;  /* active low */
sbit LCD_CE  = P2^3;  /* active low */
sbit LCD_RST = P2^4;  /* active low */

sbit LOCK_LED   = P3^0;
sbit BUZZER     = P3^1;
sbit ALARM_LED  = P3^2;
sbit LOCKED_LED = P3^3;

#define PASSWORD_LEN 6
#define LCD_COLS 16
#define TEXT_HOME 0x0000
#define GRAPH_HOME 0x0200

uchar password[PASSWORD_LEN + 1] = "123456";
uchar input_buf[PASSWORD_LEN + 1];
uchar temp_buf[PASSWORD_LEN + 1];
uchar input_len = 0;
uchar wrong_count = 0;
uint open_count = 0;

char code key_map[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

void lcd_clear_text_line(uchar row);
void lcd_put_char(uchar row, uchar col, char ch);
void lcd_puts(uchar row, uchar col, char *s);
char key_scan(void);

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

void beep_ms(uint ms)
{
    uint i;
    for (i = 0; i < ms; i++) {
        BUZZER = 0;
        delay_us(250);
        BUZZER = 1;
        delay_us(250);
    }
    BUZZER = 1;
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

void lcd_write_data(uchar dat)
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

void lcd_write_cmd(uchar cmd)
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

void lcd_write_1data_cmd(uchar data1, uchar cmd)
{
    lcd_write_data(data1);
    lcd_write_cmd(cmd);
}

void lcd_write_2data_cmd(uint value, uchar cmd)
{
    lcd_write_data((uchar)(value & 0xFF));
    lcd_write_data((uchar)(value >> 8));
    lcd_write_cmd(cmd);
}

void lcd_set_addr(uint addr)
{
    lcd_write_2data_cmd(addr, 0x24);
}

uchar lcd_char_code(char ch)
{
    if (ch < 0x20 || ch > 0x7F) {
        ch = ' ';
    }
    return (uchar)(ch - 0x20);
}

void lcd_clear(void)
{
    uint i;

    lcd_set_addr(TEXT_HOME);
    for (i = 0; i < 256; i++) {
        lcd_write_data(0x00);
        lcd_write_cmd(0xC0);
    }

    lcd_set_addr(GRAPH_HOME);
    for (i = 0; i < 2048; i++) {
        lcd_write_data(0x00);
        lcd_write_cmd(0xC0);
    }
}

void lcd_clear_text(void)
{
    uint i;

    lcd_set_addr(TEXT_HOME);
    for (i = 0; i < 256; i++) {
        lcd_write_data(0x00);
        lcd_write_cmd(0xC0);
    }
}

void lcd_clear_graph(void)
{
    uint i;

    lcd_set_addr(GRAPH_HOME);
    for (i = 0; i < 2048; i++) {
        lcd_write_data(0x00);
        lcd_write_cmd(0xC0);
    }
}

void graph_write_byte(uchar row, uchar col, uchar value)
{
    if (row >= 128 || col >= LCD_COLS) {
        return;
    }
    lcd_set_addr(GRAPH_HOME + row * LCD_COLS + col);
    lcd_write_data(value);
    lcd_write_cmd(0xC0);
}

void show_anim_frame(uchar frame)
{
    uint i;
    uint end;
    uint pos = 0;
    uint frame_size = UNLOCK_ANIM_WIDTH_BYTES * UNLOCK_ANIM_HEIGHT;
    uchar count;
    uchar value;

    if (frame >= UNLOCK_ANIM_FRAME_COUNT) {
        return;
    }

    i = unlock_anim_offsets[frame];
    end = unlock_anim_offsets[frame + 1];

    while (i < end && pos < frame_size) {
        count = unlock_anim_rle[i++];
        value = unlock_anim_rle[i++];
        while (count > 0 && pos < frame_size) {
            graph_write_byte(UNLOCK_ANIM_START_ROW + pos / UNLOCK_ANIM_WIDTH_BYTES,
                             UNLOCK_ANIM_START_COL + pos % UNLOCK_ANIM_WIDTH_BYTES,
                             value);
            pos++;
            count--;
        }
    }
}

bit star_pressed_now(void)
{
    bit pressed;

    P1 = 0xFF;
    P1 &= ~(1 << 3);       /* row 4: * 0 # D */
    delay_ms(1);
    pressed = ((P1 & 0x10) == 0);
    P1 = 0xFF;
    return pressed;
}

bit anim_wait_or_exit(uint ms)
{
    uint i;

    for (i = 0; i < ms; i += 20) {
        if (star_pressed_now()) {
            while (star_pressed_now());
            return 1;
        }
        delay_ms(20);
    }
    return 0;
}

void unlock_animation_loop(void)
{
    uchar i;

    lcd_clear_text();
    lcd_clear_graph();
    lcd_puts(1, 1, "ACCESS GRANTED");
    lcd_puts(14, 4, "* BACK");

    while (1) {
        for (i = 0; i < UNLOCK_ANIM_FRAME_COUNT; i++) {
            show_anim_frame(i);
            if (anim_wait_or_exit(30)) {
                lcd_clear_graph();
                return;
            }
        }
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

    lcd_write_2data_cmd(TEXT_HOME, 0x40);  /* text home */
    lcd_write_2data_cmd(LCD_COLS, 0x41);   /* text area */
    lcd_write_2data_cmd(GRAPH_HOME, 0x42); /* graphic home */
    lcd_write_2data_cmd(LCD_COLS, 0x43);   /* graphic area */
    lcd_write_cmd(0x80);                   /* OR mode */
    lcd_write_cmd(0x9C);                   /* text and graphic display on */
    lcd_clear();
}

void lcd_put_char(uchar row, uchar col, char ch)
{
    if (row >= 16 || col >= LCD_COLS) {
        return;
    }
    lcd_set_addr(TEXT_HOME + row * LCD_COLS + col);
    lcd_write_data(lcd_char_code(ch));
    lcd_write_cmd(0xC0);
}

void lcd_puts(uchar row, uchar col, char *s)
{
    while (*s && col < LCD_COLS) {
        lcd_put_char(row, col, *s);
        col++;
        s++;
    }
}

void lcd_put_uint4(uchar row, uchar col, uint value)
{
    lcd_put_char(row, col,     value / 1000 + '0');
    lcd_put_char(row, col + 1, value / 100 % 10 + '0');
    lcd_put_char(row, col + 2, value / 10 % 10 + '0');
    lcd_put_char(row, col + 3, value % 10 + '0');
}

void lcd_clear_text_line(uchar row)
{
    uchar col;
    if (row >= 16) {
        return;
    }

    lcd_set_addr(TEXT_HOME + row * LCD_COLS);
    for (col = 0; col < LCD_COLS; col++) {
        lcd_write_data(0x00);
        lcd_write_cmd(0xC0);
    }
}

void lcd_update_input_line(void)
{
    uchar i;

    lcd_clear_text_line(4);
    lcd_puts(4, 0, "INPUT:");
    for (i = 0; i < input_len; i++) {
        lcd_put_char(4, 6 + i, '*');
    }
}

void lcd_draw_input_char(void)
{
    if (input_len > 0 && input_len <= PASSWORD_LEN) {
        lcd_put_char(4, 5 + input_len, '*');
    }
}

void lcd_erase_input_char(void)
{
    if (input_len < PASSWORD_LEN) {
        lcd_put_char(4, 6 + input_len, ' ');
    }
}

void show_input_screen(char *title)
{
    lcd_clear_text();
    lcd_puts(0, 1, "PASSWORD LOCK");
    lcd_puts(2, 0, title);
    lcd_update_input_line();
    lcd_puts(14, 0, "*CLR CDEL #OK");
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

void clear_input(void)
{
    uchar i;
    input_len = 0;
    for (i = 0; i <= PASSWORD_LEN; i++) {
        input_buf[i] = '\0';
    }
}

bit password_equal(uchar *a, uchar *b)
{
    uchar i;
    for (i = 0; i < PASSWORD_LEN; i++) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

void copy_password(uchar *dst, uchar *src)
{
    uchar i;
    for (i = 0; i < PASSWORD_LEN; i++) {
        dst[i] = src[i];
    }
    dst[PASSWORD_LEN] = '\0';
}

void show_message(char *line1, char *line2, uint ms)
{
    lcd_clear();
    lcd_puts(4, 1, line1);
    lcd_puts(6, 1, line2);
    delay_ms(ms);
}

void unlock_action(void)
{
    LOCKED_LED = 0;
    ALARM_LED = 0;
    LOCK_LED = 1;
    open_count++;

    unlock_animation_loop();
    lcd_clear_text();
    lcd_puts(4, 1, "LOCK CLOSED");
    lcd_puts(6, 1, "BACK HOME");
    beep_ms(120);
    delay_ms(600);

    LOCK_LED = 0;
    LOCKED_LED = 1;
}

void lockout_action(void)
{
    uchar sec;
    ALARM_LED = 1;
    LOCKED_LED = 1;

    for (sec = 30; sec > 0; sec--) {
        lcd_clear();
        lcd_puts(4, 1, "LOCKOUT");
        lcd_puts(6, 1, "WAIT:");
        lcd_put_uint4(6, 7, sec);
        lcd_puts(6, 12, "S");
        beep_ms(60);
        delay_ms(940);
    }

    wrong_count = 0;
    ALARM_LED = 0;
}

bit read_password(char *title)
{
    char key;
    clear_input();
    show_input_screen(title);

    while (1) {
        key = key_scan();
        if (key == 0) {
            continue;
        }

        if (key >= '0' && key <= '9') {
            if (input_len < PASSWORD_LEN) {
                input_buf[input_len++] = key;
                input_buf[input_len] = '\0';
                lcd_draw_input_char();
            } else {
                beep_ms(50);
            }
        } else if (key == 'C') {
            if (input_len > 0) {
                input_len--;
                input_buf[input_len] = '\0';
                lcd_erase_input_char();
            }
        } else if (key == '*') {
            clear_input();
            lcd_update_input_line();
        } else if (key == '#') {
            if (input_len == PASSWORD_LEN) {
                return 1;
            }
            beep_ms(100);
            show_message("NEED 6 DIGITS", "TRY AGAIN", 800);
            show_input_screen(title);
        }
    }
}

void check_password(void)
{
    if (read_password("ENTER PASSWORD")) {
        if (password_equal(input_buf, password)) {
            wrong_count = 0;
            unlock_action();
        } else {
            wrong_count++;
            ALARM_LED = 1;
            beep_ms(300);

            if (wrong_count >= 3) {
                lockout_action();
            } else {
                show_message("PASSWORD ERR", "TRY AGAIN", 1200);
                ALARM_LED = 0;
            }
        }
    }
}

void change_password(void)
{
    if (!read_password("OLD PASSWORD")) {
        return;
    }

    if (!password_equal(input_buf, password)) {
        beep_ms(300);
        show_message("OLD PASSWORD", "ERROR", 1200);
        return;
    }

    if (!read_password("NEW PASSWORD")) {
        return;
    }
    copy_password(temp_buf, input_buf);

    if (!read_password("AGAIN NEW")) {
        return;
    }

    if (password_equal(input_buf, temp_buf)) {
        copy_password(password, temp_buf);
        wrong_count = 0;
        beep_ms(120);
        show_message("PASSWORD", "CHANGED", 1200);
    } else {
        beep_ms(300);
        show_message("NOT SAME", "CHANGE FAIL", 1200);
    }
}

void show_count(void)
{
    lcd_clear();
    lcd_puts(4, 1, "OPEN COUNT");
    lcd_put_uint4(6, 1, open_count);
    lcd_puts(14, 0, "* BACK");

    while (1) {
        if (key_scan() == '*') {
            return;
        }
    }
}

void clear_count(void)
{
    if (!read_password("ADMIN PASS")) {
        return;
    }

    if (password_equal(input_buf, password)) {
        open_count = 0;
        beep_ms(120);
        show_message("COUNT CLEAR", "OK", 1000);
    } else {
        beep_ms(300);
        show_message("ADMIN PASS", "ERROR", 1200);
    }
}

void show_home(void)
{
    lcd_clear();
    lcd_puts(0, 1, "PASSWORD LOCK");
    lcd_puts(3, 0, "# INPUT");
    lcd_puts(4, 0, "A CHANGE PASS");
    lcd_puts(5, 0, "B OPEN COUNT");
    lcd_puts(6, 0, "D CLEAR COUNT");
    lcd_puts(14, 0, "PASS:123456");
}

void show_boot_screen(void)
{
    uchar i;

    lcd_clear();
    lcd_puts(1, 2, "C51 SECURITY");
    lcd_puts(3, 2, "PASSWORD LOCK");
    lcd_puts(6, 1, "SYSTEM INIT...");
    lcd_puts(9, 1, "[              ]");

    for (i = 0; i < 14; i++) {
        lcd_put_char(9, 2 + i, '#');
        delay_ms(140);
    }

    lcd_puts(12, 5, "READY");
    delay_ms(700);
}

void main(void)
{
    char key;

    P0 = 0xFF;
    P1 = 0xFF;
    P2 = 0xFF;
    P3 = 0x00;

    LOCK_LED = 0;
    BUZZER = 1;
    ALARM_LED = 0;
    LOCKED_LED = 1;

    lcd_init();
    show_boot_screen();
    show_home();

    while (1) {
        key = key_scan();
        if (key == '#') {
            check_password();
            show_home();
        } else if (key == 'A') {
            change_password();
            show_home();
        } else if (key == 'B') {
            show_count();
            show_home();
        } else if (key == 'D') {
            clear_count();
            show_home();
        }
    }
}
