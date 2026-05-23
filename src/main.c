#include "set_up.h"

int main(void) {
    /*
        Vô hiệu hóa JTAG để giải phóng Port C.
        Không ảnh hưởng đến Port A ADC, nhưng giữ lại theo code gốc của bạn.
    */
    MCUCSR |= (1 << JTD);
    MCUCSR |= (1 << JTD);

    /*
        PB1 và PB2 làm nút chọn Sync PC.
    */
    DDRB &= ~((1 << PB1) | (1 << PB2));
    PORTB |= ((1 << PB1) | (1 << PB2));

    /*
        PB0 làm chân báo cháy: LED hoặc buzzer.
    */
    FIRE_ALARM_DDR |= (1 << FIRE_ALARM_PIN);
    FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);

    /*
        PA1 / ADC1 làm input analog đọc LM35.
        Tắt pull-up nội bộ để không kéo lệch tín hiệu analog.
    */
    DDRA &= ~(1 << PA1);
    PORTA &= ~(1 << PA1);

    LCD_Init();
    UART_Init();
    ADC_Init();

    SystemState currentState = STATE_LOADING;
    uint8_t enable_PC_Sync = 0;

    while (1) {
        switch (currentState) {
            case STATE_LOADING:
                LCD_Loading_Animation();
                currentState = STATE_ASK_PC;
                break;

            case STATE_ASK_PC:
            {
                LCD_Command(0x01);
                LCD_Command(0x80);
                LCD_String("Sync to PC?");

                for (int timeout = 50; timeout > 0; timeout--) {
                    char buf[17];

                    sprintf(buf, "Y:PB1 N:PB2 %2ds", timeout / 10);

                    LCD_Command(0xC0);
                    LCD_String(buf);

                    if (!(PINB & (1 << PB1))) {
                        _delay_ms(20);

                        if (!(PINB & (1 << PB1))) {
                            enable_PC_Sync = 1;

                            LCD_Command(0xC0);
                            LCD_String("Sync Enabled!   ");

                            _delay_ms(1000);
                            while (!(PINB & (1 << PB1)));
                            break;
                        }
                    }

                    if (!(PINB & (1 << PB2))) {
                        _delay_ms(20);

                        if (!(PINB & (1 << PB2))) {
                            enable_PC_Sync = 0;

                            LCD_Command(0xC0);
                            LCD_String("Sync Skipped!   ");

                            _delay_ms(1000);
                            while (!(PINB & (1 << PB2)));
                            break;
                        }
                    }

                    _delay_ms(100);
                }

                LCD_Command(0x01);
                currentState = STATE_RUNNING;
                break;
            }

            case STATE_RUNNING:
            {
                /*
                    Đọc ADC trung bình từ LM35 ở PA1 / ADC1.
                */
                uint16_t adc_val = ADC_Read_Avg(LM35_ADC_CHANNEL);

                /*
                    Với Vref nội bộ 2.56V:

                    Vin = ADC * 2.56 / 1024

                    LM35:
                    10mV = 0.01V tương ứng 1 độ C

                    Temp = Vin / 0.01
                         = ADC * 2.56 * 100 / 1024
                         = ADC * 256 / 1024
                         = ADC / 4

                    Để hiển thị 1 chữ số thập phân, ta tính temp_x10:

                    temp_x10 = Temp * 10
                             = ADC * 2560 / 1024
                */
                uint16_t temp_x10 = (uint16_t)((adc_val * 2550UL + 512) / 1024);

                uint16_t temp_int = temp_x10 / 10;
                uint16_t temp_dec = temp_x10 % 10;

                char display_buf[17];

                LCD_Command(0x80);
                sprintf(display_buf, "Temp:%3u.%u C    ", temp_int, temp_dec);
                LCD_String(display_buf);

                LCD_Command(0xC0);

                if (temp_x10 >= FIRE_THRESHOLD_X10) {
                    LCD_String("FIRE WARNING!   ");
                    FIRE_ALARM_PORT |= (1 << FIRE_ALARM_PIN);
                } else {
                    LCD_String("Status: Normal  ");
                    FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);
                }

                if (enable_PC_Sync) {
                    char uart_buf[40];

                    sprintf(
                        uart_buf,
                        "ADC:%u Temp:%u.%u C\r\n",
                        adc_val,
                        temp_int,
                        temp_dec
                    );

                    UART_SendString(uart_buf);
                }

                _delay_ms(500);
                break;
            }
        }
    }
    return 0;
}