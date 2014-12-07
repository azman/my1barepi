/*----------------------------------------------------------------------------*/
#ifndef __MY1UARTH__
#define __MY1UARTH__
/*----------------------------------------------------------------------------*/
#define UART_TXD_GPIO 14
#define UART_RXD_GPIO 15
/*----------------------------------------------------------------------------*/
void uart_init(void);
void uart_send(unsigned int data);
unsigned int uart_incoming(void);
unsigned int uart_read(void);
void uart_print_hex(unsigned int value);
void uart_print(char *message);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
