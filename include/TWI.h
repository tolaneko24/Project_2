#ifdef  TWI_H
#define TWI_H

// ================= NGOẠI VI GIAO TIẾP I2C (TWI) =================
void I2C_Init(void);
void I2C_Start(void);
void I2C_Write(uint8_t data);
void I2C_Stop(void);

#endif