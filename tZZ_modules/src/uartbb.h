/*----------------------------------------------------------------------------*/
#ifndef __MY1UARTBB_H__
#define __MY1UARTBB_H__
/*----------------------------------------------------------------------------*/
#define UARTBB_RX_DEFAULT 3
#define UARTBB_TX_DEFAULT 2
/*----------------------------------------------------------------------------*/
void uartbb_init(int rx_gpio,int tx_gpio);
void uartbb_send(unsigned int data);
void uartbb_find_stop(void);
unsigned int uartbb_read(void);
void uartbb_print_hex_byte(unsigned char byte);
void uartbb_print_hex_uint(unsigned int value);
void uartbb_print(char *message);
/*----------------------------------------------------------------------------*/
#endif /* __MY1UARTBB_H__ */
/*----------------------------------------------------------------------------*/
