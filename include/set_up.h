#ifndef SET_UP_H
#define SET_UP_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

// ================= CẤU HÌNH HỆ THỐNG =================
#define BAUD 9600
#define UBRR_VAL ((F_CPU/16/BAUD)-1)
#define SCL_CLOCK 100000L
#define LCD_ADDR 0x4E

// ================= CẤU HÌNH CHÂN =================
// LM35 nối vào PA1 / ADC1
#define LM35_ADC_CHANNEL        1
#define GAS_ADC_CHANNEL         2
#define MOISTURE_ADC_CHANNEL    3

// Cảm biến ánh sáng / ngọn lửa (Kết nối Port B)
#define LIGHT_SENSOR_PORT       PORTB
#define LIGHT_SENSOR_PIN        PINB
#define LIGHT_SENSOR_DDR        DDRB
#define LIGHT_SENSOR_BIT        PB3

// Ampli LM386 phát nhạc báo cháy nối PD5 (Chân phần cứng OC1A của Timer1)
#define FIRE_ALARM_PORT PORTD
#define FIRE_ALARM_DDR  DDRD
#define FIRE_ALARM_PIN  PD5

// Ngưỡng báo cháy
#define FIRE_THRESHOLD_X10      400  // 40.0 độ C
#define GAS_DANGER_THRESHOLD    600  // Mức ADC cảnh báo nồng độ khí cháy
#define GAS_WARMUP_TIME         120 // 120 chu kỳ 500ms = 60 giây
// ================= MÁY TRẠNG THÁI HỮU HẠN =================
typedef enum {
    STATE_LOADING,
    STATE_RUNNING
} SystemState;

#endif