#include "playmusic.h"

// ================= NGOẠI VI ÂM THANH (HARDWARE PWM - CTC MODE) =================
const Note_t HappyBirthday_Melody[]= {

// Khúc 1: "I do the same thing I told you that I never would"
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_G5, 176}, {NOTE_C5, 176}, 
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_G4, 353}, {NOTE_REST, 176},
    
    // Khúc 2: "I told you I'd change, even when I knew I never could"
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_G5, 176}, {NOTE_C5, 176}, 
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_G4, 176}, {NOTE_A4, 353}, {NOTE_REST, 176},
    
    // Khúc 3: "Know that I can't find nobody else as good as you"
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_G5, 176}, {NOTE_C5, 176}, 
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_G4, 353}, {NOTE_REST, 353},
    
    // Khúc 4: "I need you to stay, need you to stay, hey"
    {NOTE_B4, 176}, {NOTE_C5, 176}, {NOTE_B4, 176}, {NOTE_G4, 176}, 
    {NOTE_A4, 529}, {NOTE_REST, 353},
    
    // Khúc 5: Đoạn chuyển nhịp nhanh (Mật độ 88ms/nốt)
    {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_G5, 529}, {NOTE_REST, 176},
    {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  
    {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  
    {NOTE_C5, 176}, {NOTE_C5, 353}, {NOTE_REST, 176},
    
    // Khúc 6: Lặp lại âm hình tuyến tính
    {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_G5, 529}, {NOTE_REST, 176},
    {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  
    {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  
    {NOTE_C5, 176}, {NOTE_C5, 353}, {NOTE_REST, 176},
    
    // Khúc 7: Lặp lại âm hình tuyến tính
    {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_G5, 529}, {NOTE_REST, 176},
    {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  
    {NOTE_B4, 88},  {NOTE_B4, 88},  {NOTE_B4, 88},  
    {NOTE_C5, 176}, {NOTE_C5, 353}, {NOTE_REST, 176},
    
    // Khúc 8: Đoạn Outro dồn nhịp
    {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_C5, 176}, {NOTE_G5, 529}, {NOTE_REST, 176},
    {NOTE_C5, 176}, {NOTE_D5, 176}, {NOTE_D5, 176}, {NOTE_B5, 176}, 
    {NOTE_G5, 176}, {NOTE_A5, 706}
};

#define MELODY_LENGTH (sizeof(HappyBirthday_Melody) / sizeof(Note_t))

void Audio_Init(void) {
    // Cấu hình chân PD5 (OC1A) làm ngõ ra tín hiệu âm thanh
    FIRE_ALARM_DDR |= (1 << FIRE_ALARM_PIN);
    FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);

    /*
        Cấu hình Timer1 ở chế độ CTC (Clear Timer on Compare Match)
        Bit WGM12 = 1 đưa Timer1 vào Mode 4.
        Bit COM1A0 = 1 ra lệnh cho phần cứng TỰ ĐỘNG lật mức logic (Toggle) 
        tại chân vật lý OC1A (PD5) mỗi khi bộ đếm đạt ngưỡng.
    */
    TCCR1A = (1 << COM1A0);
    TCCR1B = (1 << WGM12); 
    // Mặc định Timer1 đang dừng do chưa cấp xung nhịp (Prescaler = 0)
}

void Delay_ms_Custom(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);
    }
}

void Play_Tone(uint16_t frequency, uint16_t duration) {
    if (frequency == NOTE_REST) {
        // Tắt bộ đếm Timer1 (Ngắt xung nhịp CS12:0 = 000)
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        
        // Buộc chân PD5 về mức thấp để triệt tiêu dòng một chiều vào Ampli
        TCCR1A &= ~(1 << COM1A0); 
        FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);
        
        Delay_ms_Custom(duration);
    } else {
        // Khôi phục quyền điều khiển chân PD5 cho khối Timer phần cứng
        TCCR1A |= (1 << COM1A0);

        // Tính toán ngưỡng giá trị nạp vào thanh ghi so sánh
        uint16_t ocr_value = (uint16_t)(62500UL / frequency) - 1;
        
        // Nạp giá trị 16-bit vào thanh ghi OCR1A
        OCR1AH = (ocr_value >> 8);
        OCR1AL = (ocr_value & 0xFF);
        
        // Đặt lại bộ đếm để ngăn chặn sai lệch pha của sóng âm
        TCNT1H = 0;
        TCNT1L = 0;
        
        // Khởi động Timer1 với bộ chia Prescaler = 64 (Bit CS11=1, CS10=1)
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        TCCR1B |= (1 << CS11) | (1 << CS10);
        
        // Vi điều khiển chỉ việc chờ, toàn bộ việc phát âm thanh do phần cứng tự làm
        uint16_t play_time = duration * 9 / 10;
        uint16_t gap_time  = duration - play_time;
        Delay_ms_Custom(play_time);
   
        // Kết thúc nốt nhạc: Dừng Timer1 và thu hồi chân PD5 về mức 0
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        TCCR1A &= ~(1 << COM1A0);
        FIRE_ALARM_PORT &= ~(1 << FIRE_ALARM_PIN);
        Delay_ms_Custom(gap_time);
    }
}

void Play_Alarm_Melody(void) {
    for (uint16_t i = 0; i < MELODY_LENGTH; i++) {
        // Trích xuất dữ liệu 16-bit (word) từ địa chỉ vùng nhớ Flash
        uint16_t current_freq = HappyBirthday_Melody[i].frequency;
        uint16_t current_dur  = HappyBirthday_Melody[i].duration;
    
        // Gọi hàm xuất tín hiệu PWM
        Play_Tone(current_freq, current_dur);
        
        // Mô phỏng thông số NOTE_GAP_MS = 8 từ tập lệnh Python
        if (current_freq != NOTE_REST) {
            Delay_ms_Custom(8); 
        }
    }
}