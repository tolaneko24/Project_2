#include "set_up.h"
#include "ADC.h"
#include "LCD_I2C.h"
#include "Loading_ani.h"
#include "TWI.h"
#include "UART.h"
#include "music_node.h"
#include "playmusic.h"

int main(void) {

    // Vô hiệu hóa JTAG để giải phóng Port / Đã vô hiệu hóa bằng FuseBit nhưng vẫn cẩn trọng
    MCUCSR |= (1 << JTD);
    MCUCSR |= (1 << JTD);

    /*
        PB1 làm nút chọn Sync PC
        PB2 làm nút phát xung PWM ->> Audio
        PB3 làm ngõ vào Digital đọc cảm biến ánh sáng
    */
    DDRB &= ~((1 << PB1) | (1 << PB2) | (1 << LIGHT_SENSOR_BIT));
    PORTB |= ((1 << PB1) | (1 << PB2) | (1 << LIGHT_SENSOR_BIT));

    // PD5 làm chân báo cháy: LM385 ->> Loa
    FIRE_ALARM_DDR |= (1 << FIRE_ALARM_PIN);
    FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);


    // PA1, PA2, PA3 làm input analog đọc LM35, Khí Gas, Độ ẩm.
    DDRA &= ~((1 << PA1) | (1 << PA2) | (1 << PA3));
    PORTA &= ~((1 << PA1) | (1 << PA2) | (1 << PA3));

    LCD_Init();
    UART_Init();
    ADC_Init();
    Audio_Init();
    
    SystemState currentState = STATE_LOADING;
    uint8_t enable_PC_Sync = 0;
    uint8_t task_timer = 50;

    while (1) {
        switch (currentState) {

            case STATE_LOADING:
            {
                // LCD_Loading_Animation();
                currentState = STATE_RUNNING;
                break;
            }

            case STATE_RUNNING:
            {
                if (!(PINB & (1 << PB1))) {
                    _delay_ms(20); // Chống dội phím (Debounce)
                    if (!(PINB & (1 << PB1))) {
                        
                        // Đảo trạng thái logic (Toggle) của biến đồng bộ
                        enable_PC_Sync = !enable_PC_Sync; 
                        LCD_Command(0x01); 
                        LCD_Command(0x80);
                        if (enable_PC_Sync) {
                            LCD_String("UART Sync: ON   ");
                        } else {
                            LCD_String("UART Sync: OFF  ");
                        }
                        _delay_ms(2000);
                        while (!(PINB & (1 << PB1))); // Khóa luồng chờ nhả phím
                        task_timer = 50; // Ép cập nhật lại màn hình chính ngay lập tức
                    }
                }

                // Phím PB2: Kích hoạt phát bản nhạc thủ công
                if (!(PINB & (1 << PB2))) {
                    _delay_ms(20); 
                    if (!(PINB & (1 << PB2))) {
                        
                        LCD_Command(0x01);
                        LCD_Command(0x80);
                        LCD_String("Playing Music");
                        
                        // Hàm điều phối âm thanh hoạt động theo cơ chế chặn luồng (Blocking)
                        Play_Alarm_Melody(); 
                        
                        while (!(PINB & (1 << PB2))); 
                        task_timer = 50; 
                    }
                }

                // Khối lệnh này chỉ thực thi khi bộ đếm đạt 50 chu kỳ (Tương đương 500ms)
                if (task_timer >= 50) {
                    task_timer = 0; // Đặt lại bộ đếm    

                // Đọc ADC trung bình từ LM35 ở PA1 / ADC1.
                // Lấy mẫu giá trị ADC trung bình cảm biến nhiệt độ
                uint16_t adc_val = ADC_Read_Avg(LM35_ADC_CHANNEL);

                // Bù sai số tĩnh
                if (adc_val > 1) {
                    adc_val = adc_val - 1; 
                } else {
                    adc_val = 0; // Tránh hiện tượng tràn
                }

                // Biến đếm x10 để hiển thị số nguyên nhằm lấy số sau dấu phẩy
                uint16_t temp_x10 = (uint16_t)((adc_val * 2560UL + 512) / 1024);

                // Trích xuất phần nguyên và phần thập phân
                uint16_t temp_int = temp_x10 / 10;
                uint16_t temp_dec = temp_x10 % 10;
                
                _delay_ms(5);

                // Lấy mẫu đa kênh các cảm biến mở rộng
                uint16_t adc_moisture = ADC_Read_Avg(MOISTURE_ADC_CHANNEL);
                uint8_t humid_percent = (uint8_t)(100UL - (adc_moisture * 100UL) / 1024);

                // Đọc trạng thái Digital từ LM393 (Active-Low)
                uint8_t is_bright = !(LIGHT_SENSOR_PIN & (1 << LIGHT_SENSOR_BIT));
             
                char display_buf[17];

                uint8_t gas_warmup_ticks = 0;

                uint16_t adc_gas = ADC_Read_Avg(GAS_ADC_CHANNEL);

                // Khối logic cập nhật thời gian sấy cảm biến Gas
                uint8_t gas_is_ready = 0;
                if (gas_warmup_ticks < GAS_WARMUP_TIME) {
                    gas_warmup_ticks++;
                    // Đang trong giai đoạn sấy, bỏ qua xử lý logic cảnh báo
                } else {
                    gas_is_ready = 1; 
                }

            /*

                Kết hợp cảm biến có điều kiện an toàn
                Hệ thống chỉ xét ngưỡng ADC Gas khi biến gas_is_ready được xác lập = 1

            */
                if (((temp_x10 >= FIRE_THRESHOLD_X10) && (is_bright == 1)) || 
                (gas_is_ready && (adc_gas >= GAS_DANGER_THRESHOLD))) {
                    
                    // Kích hoạt chuỗi còi báo động khẩn cấp
                    LCD_Command(0x01);
                    LCD_Command(0x80);
                    LCD_String("CRITICAL DANGER!");
                    Play_Alarm_Melody();

                } else {

                    // Cập nhật giao diện giám sát 4 thông số
                    LCD_Command(0x80);
                    sprintf(display_buf, "T:%2u.%uC Hum: %2u%%", temp_int, temp_dec, humid_percent);
                    LCD_String(display_buf);

                    LCD_Command(0xC0);
                    sprintf(display_buf, "Light:%u Gas:%-4u", is_bright, adc_gas);
                    LCD_String(display_buf);
                    
                    // Cơ chế khóa an toàn: Cắt hoàn toàn xung nhịp và thu hồi quyền điều khiển chân PD5
                    // Đảm bảo Ampli LM386 không bị phát tiếng rè nhiễu.
                    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
                    TCCR1A &= ~(1 << COM1A0);
                    FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);
                }

                // Truyền data qua (USB to UART) đến cổng COM của PC
                if (enable_PC_Sync) {
                    char uart_buf[50];

                    sprintf(
                        uart_buf,
                        "T:%u.%u H:%u%% L:%u G:%u\r\n",
                        temp_int,
                        temp_dec,
                        humid_percent,
                        is_bright,
                        adc_gas
                    );
                    UART_SendString(uart_buf);
                }
            }
                _delay_ms(10);
                task_timer++;
                break;
            }
            default: currentState = STATE_LOADING;
            break;
        }
    }
    return 0;
}