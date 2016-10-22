/*----------------------------------------------------------------------------*/
#ifndef __MY1UARTBBH__
#define __MY1UARTBBH__
/*----------------------------------------------------------------------------*/
#define UARTBB_RX_DEFAULT 3
#define UARTBB_TX_DEFAULT 2
/*----------------------------------------------------------------------------*/
void uartbb_init(int rx_gpio,int tx_gpio);
void uartbb_send(unsigned int data);
void uartbb_find_stop(void);
unsigned int uartbb_read(void);
void uartbb_print_hex(unsigned int value);
void uartbb_print(char *message);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
