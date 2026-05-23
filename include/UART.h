#ifdef  UART_H
#define UART_H

// ================= NGOẠI VI USART (UART) =================
void UART_Init(void);
void UART_TxChar(char data);
void UART_SendString(char *str);

#endif
