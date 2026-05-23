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
#define LM35_ADC_CHANNEL 1

// Buzzer hoặc LED báo cháy nối PB0
#define FIRE_ALARM_PORT PORTB
#define FIRE_ALARM_DDR  DDRB
#define FIRE_ALARM_PIN  PB0

// Ngưỡng báo cháy: 50.0 độ C
// Vì ta dùng temp_x10, 50.0 C = 500
#define FIRE_THRESHOLD_X10 330



// ================= NGOẠI VI ADC =================
void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);
uint16_t ADC_Read_Avg(uint8_t channel);

// ================= MÁY TRẠNG THÁI HỮU HẠN =================
typedef enum {
    STATE_LOADING,
    STATE_ASK_PC,
    STATE_RUNNING
} SystemState;

#endif