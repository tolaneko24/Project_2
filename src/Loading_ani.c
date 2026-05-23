#include "set_up.h"
#include "Loading_ani.h"
#include "LCD_I2C.h"

// ================= GIAO DIỆN KHỞI ĐỘNG =================
void LCD_Loading_Animation(void) {
    char buffer[17];

    for (int i = 0; i <= 100; i++) {
        sprintf(buffer, "Load %3d%%", i);

        LCD_Command(0x80);
        LCD_String(buffer);

        LCD_Command(0xC0);

        for (int j = 0; j < 16; j++) {
            if (j < (i * 16) / 100)
                LCD_Char(0xFF);
            else
                LCD_Char(0x20);
        }

        _delay_ms(30);
    }

    LCD_Command(0x01);
}