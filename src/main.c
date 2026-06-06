#include "set_up.h"
#include "ADC.h"
#include "LCD_I2C.h"
#include "Loading_ani.h"
#include "TWI.h"
#include "UART.h"
#include "music_node.h"
#include "playmusic.h"

#define MASK_DDRB ((1 << PB1) | (1 << PB2) | (1 << LIGHT_SENSOR_BIT))
#define MASK_PORTB ((1 << PB1) | (1 << PB2) | (1 << LIGHT_SENSOR_BIT))
#define MASK_DDRA ((1 << PA1) | (1 << PA2) | (1 << PA3))
#define MASK_PORTA ((1 << PA1) | (1 << PA2) | (1 << PA3))
#define clear_display 0x01
#define set_pointer_1 0x80
#define set_pointer_2 0xC0
#define MASK_SAFE_KEY ((1 << CS12) | (1 << CS11) | (1 << CS10))

int main(void) {

    // Vô hiệu hóa JTAG để giải phóng Port / Đã vô hiệu hóa bằng FuseBit nhưng vẫn cẩn trọng
    set_bit(MCUCSR, JTD);
    set_bit(MCUCSR, JTD);

    /*
        PB1 làm nút chọn Sync PC
        PB2 làm nút phát xung PWM ->> Audio
        PB3 làm ngõ vào Digital đọc cảm biến ánh sáng
    */
    clear_bit_mask(DDRB, MASK_DDRB);
    set_bit_mask(PORTB, MASK_PORTB);

    // PD5 làm chân báo cháy: LM385 ->> Loa
    set_bit(FIRE_ALARM_DDR, FIRE_ALARM_PIN);
    clear_bit(FIRE_ALARM_PORT, FIRE_ALARM_PIN);

    // PA1, PA2, PA3 làm input analog đọc LM35, Khí Gas, Độ ẩm.
    clear_bit_mask(DDRA, MASK_DDRA);
    clear_bit_mask(PORTA, MASK_PORTA);

    LCD_Init();
    UART_Init();
    ADC_Init();
    Audio_Init();
    
    SystemState currentState = STATE_LOADING;
    uint8_t enable_PC_Sync = 0;
    uint8_t task_timer = 50;

    // Khai báo các biến quản lý trạng thái nút bấm (Phi chặn)
    uint8_t pb1_last_state = 1; 
    uint8_t pb2_last_state = 1;
    uint16_t sync_display_timeout = 0; 

    // Tối ưu hóa bộ nhớ: Khai báo mảng đệm một lần duy nhất
    char display_buf[17];
    char uart_buf[50];

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
                // ==================== LOGIC NÚT BẤM PHI CHẶN ====================
                // Đọc trạng thái vật lý tức thời (1 = Nhả phím, 0 = Nhấn phím do kéo lên)
                uint8_t pb1_current_state = (check_bit(PINB, PB1) != 0);
                uint8_t pb2_current_state = (check_bit(PINB, PB2) != 0);

                // Nút PB1: Phát hiện sườn xuống (Chuyển từ 1 -> 0)
                if ((pb1_last_state == 1) && (pb1_current_state == 0)) {
                    enable_PC_Sync = !enable_PC_Sync; 
                    
                    LCD_Command(clear_display); 
                    LCD_Command(set_pointer_1);

                    if (enable_PC_Sync) {
                        LCD_String("UART Sync: ON   ");
                    } else {
                        LCD_String("UART Sync: OFF  ");
                    }

                    // Khóa giao diện LCD trong 200 chu kỳ (200 * 10ms = 2000ms)
                    sync_display_timeout = 200; 
                }
                pb1_last_state = pb1_current_state; 

                // Nút PB2: Phát hiện sườn xuống
                if ((pb2_last_state == 1) && (pb2_current_state == 0)) {
                    LCD_Command(clear_display);
                    LCD_Command(set_pointer_1);
                    LCD_String("Playing Music");
                    
                    Play_Alarm_Melody(); 
                    
                    // Ép cập nhật lại màn hình chính ngay sau khi phát xong
                    task_timer = 50; 
                }
                pb2_last_state = pb2_current_state;

                // Xử lý bộ định thời đếm ngược cho giao diện UART Sync
                if (sync_display_timeout > 0) {
                    sync_display_timeout--;
                    if (sync_display_timeout == 0) {
                        task_timer = 50; // Ép cập nhật màn hình ngay lập tức khi hết 2 giây
                    }
                }
                // ================================================================

                // Khối lệnh này chỉ thực thi khi bộ đếm đạt 50 chu kỳ (Tương đương 500ms)
                if (task_timer >= 50) {
                    task_timer = 0; // Đặt lại bộ đếm    

                    // Đọc ADC trung bình từ LM35 ở PA1 / ADC1.
                    uint16_t adc_val = ADC_Read_Avg(LM35_ADC_CHANNEL);

                    // Bù sai số tĩnh
                    if (adc_val > 1) {
                        adc_val = adc_val - 1; 
                    } else {
                        adc_val = 0; 
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
                    uint8_t is_bright = !check_bit(LIGHT_SENSOR_PIN, LIGHT_SENSOR_BIT);
                 
                    static uint8_t gas_warmup_ticks = 0;
                    uint16_t adc_gas = ADC_Read_Avg(GAS_ADC_CHANNEL);
                    uint8_t gas_is_ready = 0;

                    // Khối logic cập nhật thời gian sấy cảm biến Gas
                    if (gas_warmup_ticks < GAS_WARMUP_TIME) {
                        gas_warmup_ticks++;
                    } else {
                        gas_is_ready = 1; 
                    }

                    // Khối kiểm tra điều kiện an toàn
                    if (((temp_x10 >= FIRE_THRESHOLD_X10) && (is_bright == 1)) || 
                        (gas_is_ready && (adc_gas >= GAS_DANGER_THRESHOLD))) {
                        
                        // Mutex Override: Xóa bỏ trạng thái chờ UART để nhường quyền ưu tiên cho báo cháy
                        sync_display_timeout = 0;
                        
                        // Kích hoạt chuỗi còi báo động khẩn cấp
                        LCD_Command(clear_display);
                        LCD_Command(set_pointer_1);
                        LCD_String("CRITICAL DANGER!");
                        Play_Alarm_Melody();

                    } else {
                        // Cơ chế Mutex: Chỉ cập nhật LCD giám sát khi không có thông báo UART Sync nào đang chờ
                        if (sync_display_timeout == 0) {
                            LCD_Command(set_pointer_1);
                            sprintf(display_buf, "T:%2u.%uC H: %2u%%", temp_int, temp_dec, humid_percent);
                            LCD_String(display_buf);

                            LCD_Command(set_pointer_2);
                            sprintf(display_buf, "Light:%u Gas:%-4u", is_bright, adc_gas);
                            LCD_String(display_buf);
                        }
                        
                        // Cơ chế khóa an toàn: Cắt hoàn toàn xung nhịp và thu hồi quyền điều khiển chân PD5
                        clear_bit_mask(TCCR1B, MASK_SAFE_KEY);
                        clear_bit(TCCR1A, COM1A0);
                        clear_bit(FIRE_ALARM_PORT, FIRE_ALARM_PIN);
                    }

                    // Truyền data qua (USB to UART) đến cổng COM của PC
                    if (enable_PC_Sync) {
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

                // Bộ định thời gốc (10ms / chu kỳ)
                _delay_ms(10);
                task_timer++;
                break;
            }
            default: 
                currentState = STATE_LOADING;
                break;
        }
    }
    return 0;
}