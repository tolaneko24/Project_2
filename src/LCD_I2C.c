#include "set_up.h"
#include "LCD_I2C.h"

// ================= KHỐI ĐIỀU KHIỂN LCD I2C =================
void LCD_Write_Nibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble & 0xF0) | (rs & 0x01) | 0x08;

    I2C_Start();
    I2C_Write(LCD_ADDR);
    I2C_Write(data | 0x04);
    _delay_us(1);
    I2C_Write(data & ~0x04);
    I2C_Stop();

    _delay_us(50);
}

void LCD_Write_Byte(uint8_t data, uint8_t rs) {
    LCD_Write_Nibble(data & 0xF0, rs);
    LCD_Write_Nibble((data << 4) & 0xF0, rs);
}

void LCD_Command(uint8_t cmd) {
    LCD_Write_Byte(cmd, 0);
    _delay_ms(2);
}

void LCD_Char(uint8_t data) {
    LCD_Write_Byte(data, 1);
}

void LCD_String(char *str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

void LCD_Init(void) {
    I2C_Init();

    _delay_ms(50);

    LCD_Write_Nibble(0x30, 0);
    _delay_ms(5);

    LCD_Write_Nibble(0x30, 0);
    _delay_ms(1);

    LCD_Write_Nibble(0x30, 0);
    _delay_ms(1);

    LCD_Write_Nibble(0x20, 0);
    _delay_ms(1);

    LCD_Command(0x28);
    LCD_Command(0x0C);
    LCD_Command(0x06);
    LCD_Command(0x01);
}