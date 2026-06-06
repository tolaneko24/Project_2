#include "TWI.h"
#include "set_up.h"

#define SCL_baudrate (((F_CPU / SCL_CLOCK) - 16 ) / 2)
#define MASK_START ((1 << TWINT) | (1 << TWSTA) | (1 << TWEN))
#define MASK_WRITE ((1 << TWINT) | (1 << TWEN))
#define MASK_STOP ((1 << TWINT) | (1 << TWSTO) | (1 << TWEN))

// ================= NGOẠI VI GIAO TIẾP I2C (TWI) =================
void I2C_Init(void) {

    write_reg(TWSR, 0x00);
    write_reg(TWBR, SCL_baudrate);
    write_reg(TWCR, TWEN);

}

void I2C_Start(void) {

    write_reg(TWCR, MASK_START) ;
    while (!check_bit(TWCR, TWINT));

}

void I2C_Write(uint8_t data) {

    write_reg(TWDR, data);
    write_reg(TWCR, MASK_WRITE);
    while (!check_bit(TWCR, TWINT));

}

void I2C_Stop(void) {

    write_reg(TWCR, MASK_STOP);
    _delay_us(10);

}
