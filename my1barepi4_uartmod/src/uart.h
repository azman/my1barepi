/*----------------------------------------------------------------------------*/
#ifndef __MY1UARTH__
#define __MY1UARTH__
/*----------------------------------------------------------------------------*/
#define UART_TXD_GPIO 14
#define UART_RXD_GPIO 15
/*----------------------------------------------------------------------------*/
#define UART_BAUD_115200 270
#define UART_BAUD_9600 3254
#define UART_BAUD_DEFAULT UART_BAUD_9600
/*----------------------------------------------------------------------------*/
void uart_init(int baudrate);
void uart_send(unsigned int data);
unsigned int uart_incoming(void);
unsigned int uart_read(void);
void uart_print_hex(unsigned int value);
void uart_print(char *message);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
