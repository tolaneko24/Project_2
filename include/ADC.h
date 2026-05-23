#ifndef ADC_H
#define ADC_H


// ================= NGOẠI VI ADC =================
void ADC_Init(void);
uint16_t ADC_Read(uint8_t channel);
uint16_t ADC_Read_Avg(uint8_t channel);

#endif