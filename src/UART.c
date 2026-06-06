#include "UART.h"
#include "set_up.h"

#define BIT_H_UART ((unsigned char)(UBRR_VAL >> 8)) 
#define BIT_L_UART ((unsigned char)UBRR_VAL)
#define MASK_FRAME ((1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0))

// ================= NGOẠI VI USART (UART) =================
void UART_Init(void) {

    write_reg(UBRRH, BIT_H_UART);
    write_reg(UBRRL, BIT_L_UART);

    write_reg(UCSRB, TXEN);
    write_reg(UCSRC, MASK_FRAME);

}

void UART_TxChar(char data) {

    while (!check_bit(UCSRA, UDRE));
    write_reg(UDR, data);

}

void UART_SendString(char *str) {

    while (*str) {
        UART_TxChar(*str++);
    }

}






