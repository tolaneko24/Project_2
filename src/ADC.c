#include "set_up.h"
#include "ADC.h"

// ================= NGOẠI VI ADC =================
void ADC_Init(void) {
    /*
        Dùng điện áp tham chiếu nội bộ 2.56V.

        Với ATmega16:
        REFS1 = 1, REFS0 = 1  => Điện áp tham chiếu 2.56V

        ADLAR = 0             => đọc kết quả 10-bit bình thường trong ADCW
        MUX bits = 0000       => ban đầu chọn ADC0, sau đó ADC_Read sẽ đổi kênh
    */
    ADMUX = (1 << REFS1) | (1 << REFS0);

    /*
        Prescaler = 64.
        F_CPU = 8MHz
        ADC clock = 8MHz / 64 = 125kHz
    */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

    _delay_ms(5);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
}

uint16_t ADC_Read(uint8_t channel) {

    ADMUX = (ADMUX & 0xE0) | (channel & 0x07);
    _delay_us(50);

    /*
        Đọc bỏ một lần sau khi chọn kênh.
    */
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADCW;
}

// Filter phần mềm
uint16_t ADC_Read_Avg(uint8_t channel) {
    uint32_t sum = 0;

    for (uint8_t i = 0; i < 100; i++) {
        sum += ADC_Read(channel);
        _delay_ms(2);
    }

    return (uint16_t)(sum / 100);
}