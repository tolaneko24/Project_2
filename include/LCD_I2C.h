#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <avr/io.h>

void LCD_Write_Nibble(uint8_t nibble, uint8_t rs);
void LCD_Write_Byte(uint8_t data, uint8_t rs);
void LCD_Command(uint8_t cmd);
void LCD_Char(uint8_t data);
void LCD_String(char *str);
void LCD_Init(void);

#endif