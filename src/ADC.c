#include "set_up.h"
#include "ADC.h"

/*
    Thanh ghi:

    ADMUX: Thanh ghi dồn kênh và chọn điện áp tham chiếu.
    ADCSRA: Thanh ghi điều khiển và trạng thái A của khối ADC.
    ADCW: Macro tự động gộp thanh ghi ADCH (High) và ADCL (Low) để trả về giá trị 16-bit.

    Bit Position:
    
    REFS1, REFS0: Bit chọn nguồn điện áp tham chiếu (Reference Selection).
    ADEN: Bit cấp nguồn cho module ADC (ADC Enable).
    ADSC: Bit kích hoạt quá trình chuyển đổi (Start Conversion).
    ADPS2, ADPS1: Các bit chia tần số xung nhịp cho khối ADC (Prescaler).

*/
#define MASK_Vref (1 << REFS1) | (1 << REFS0)
#define MASK_ADCSRA (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1)
#define MASK_channel (channel & 0x07)

// ================= NGOẠI VI ADC =================
void ADC_Init(void) {
    /*
        Dùng điện áp tham chiếu nội bộ 2.56V.

        Với ATmega16:
        REFS1 = 1, REFS0 = 1  => Điện áp tham chiếu 2.56V

        ADLAR = 0             => đọc kết quả 10-bit bình thường trong ADCW
        MUX bits = 0000       => ban đầu chọn ADC0, sau đó ADC_Read sẽ đổi kênh
    */
    write_reg(ADMUX, MASK_Vref);
    /*
        Prescaler = 64.
        F_CPU = 8MHz
        ADC clock = 8MHz / 64 = 125kHz
    */
    write_reg(ADCSRA, MASK_ADCSRA);
    _delay_ms(5);

    set_bit(ADCSRA, ADSC);
    while (check_bit(ADCSRA, ADSC));
}

uint16_t ADC_Read(uint8_t channel) {

    insert_mask(ADMUX, 0xE0, MASK_channel);
    _delay_us(50);

    /*
        Đọc bỏ một lần sau khi chọn kênh.
    */
    set_bit(ADCSRA, ADSC);
    while (check_bit(ADCSRA, ADSC));

    // Lần đọc thực tế
    set_bit(ADCSRA, ADSC);
    while (check_bit(ADCSRA, ADSC));

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