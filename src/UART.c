#include "UART.h"
#include "set_up.h"

// ================= NGOẠI VI USART (UART) =================
void UART_Init(void) {
    UBRRH = (unsigned char)(UBRR_VAL >> 8);
    UBRRL = (unsigned char)UBRR_VAL;

    UCSRB = (1 << TXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

void UART_TxChar(char data) {
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

void UART_SendString(char *str) {
    while (*str) {
        UART_TxChar(*str++);
    }
}






