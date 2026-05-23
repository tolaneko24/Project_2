#include "set_up.h"
#include "ADC.h"

// ================= NGOẠI VI ADC =================
void ADC_Init(void) {
    /*
        Dùng điện áp tham chiếu nội bộ 2.56V.

        Với ATmega16:
        REFS1 = 1, REFS0 = 1  => Internal 2.56V Voltage Reference

        ADLAR = 0             => đọc kết quả 10-bit bình thường trong ADCW
        MUX bits = 0000       => ban đầu chọn ADC0, sau đó ADC_Read sẽ đổi kênh
    */
    ADMUX = (1 << REFS1) | (1 << REFS0);

    /*
        Bật ADC.
        Prescaler = 64.

        F_CPU = 8MHz
        ADC clock = 8MHz / 64 = 125kHz
        Đây là vùng phù hợp để ADC hoạt động ổn định.
    */
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);

    /*
        Chờ Vref nội bộ ổn định một chút.
        Sau đó đọc bỏ một lần đầu.
    */
    _delay_ms(5);

    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
}

uint16_t ADC_Read(uint8_t channel) {
    /*
        Giữ nguyên REFS1, REFS0.
        Chỉ thay đổi MUX để chọn kênh ADC.

        channel = 1 nghĩa là đọc PA1 / ADC1.
    */
    ADMUX = (ADMUX & 0xE0) | (channel & 0x07);

    /*
        Cho bộ chọn kênh analog MUX ổn định.
    */
    _delay_us(50);

    /*
        Đọc bỏ một lần sau khi chọn kênh.
        Việc này giúp kết quả ổn định hơn.
    */
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    /*
        Đọc thật.
    */
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADCW;
}

uint16_t ADC_Read_Avg(uint8_t channel) {
    uint32_t sum = 0;

    for (uint8_t i = 0; i < 100; i++) {
        sum += ADC_Read(channel);
        _delay_ms(2);
    }

    return (uint16_t)(sum / 100);
}