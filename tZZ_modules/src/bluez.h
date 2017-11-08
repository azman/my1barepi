/*----------------------------------------------------------------------------*/
#ifndef __MY1BLUEZ_H
#define __MY1BLUEZ_H
/*----------------------------------------------------------------------------*/
/** HC-06 (slave-only) module
 *  - baudrate always 9600?
***/
/*----------------------------------------------------------------------------*/
#define BT_NAME_BUFF 32
#define BT_NAME_SIZE 20
#define BT_CPIN_BUFF 8
#define BT_CPIN_SIZE 4
#define BT_BUFF_SIZE 32
/*----------------------------------------------------------------------------*/
typedef struct _btmodule_t
{
	int status, bbsize;
	char name[BT_NAME_BUFF];
	char cpin[BT_CPIN_BUFF];
	char vers[BT_BUFF_SIZE];
	unsigned char buff[BT_BUFF_SIZE];
}
btmodule_t;
/*----------------------------------------------------------------------------*/
int bt_replies(btmodule_t* btinfo);
int bt_cmdwait(btmodule_t* btinfo);
void bt_init(btmodule_t* btinfo);
void bt_send(unsigned int data);
void bt_print(char *message);
unsigned int bt_scan(void);
unsigned int bt_read(btmodule_t* btinfo);
void bt_purge(void);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
