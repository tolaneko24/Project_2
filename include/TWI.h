#ifndef TWI_H
#define TWI_H

#include <avr/io.h>
// ================= NGOẠI VI GIAO TIẾP I2C (TWI) =================
void I2C_Init(void);
void I2C_Start(void);
void I2C_Write(uint8_t data);
void I2C_Stop(void);

#endif