/*----------------------------------------------------------------------------*/
#include "raspi.h"
/*----------------------------------------------------------------------------*/
#define MAILBOX_BASE (PMAP_BASE|MAILBOX_OFFSET)
/*----------------------------------------------------------------------------*/
#include "mailbox.h"
/*----------------------------------------------------------------------------*/
#define MAIL0_BASE    0x0
#define MAIL0_READ    0x0
#define MAIL0_POLL    0x4
#define MAIL0_SEND_ID 0x5
#define MAIL0_STATUS  0x6
#define MAIL0_CONFIG  0x7
#define MAIL0_WRITE   0x8
/* MAIL0_WRITE IS ACTUALLY MAIL1_BASE? */
#define MAIL1_BASE    0x8
#define MAIL1_READ    0x8
#define MAIL1_STATUS  0xE
/*----------------------------------------------------------------------------*/
#define MAIL_STATUS_FULL  0x80000000
#define MAIL_STATUS_EMPTY 0x40000000
/*----------------------------------------------------------------------------*/
volatile unsigned int *mailbox;
/*----------------------------------------------------------------------------*/
void mailbox_init(void)
{
	mailbox = (unsigned int*) MAILBOX_BASE;
}
/*----------------------------------------------------------------------------*/
unsigned int mailbox_read(unsigned int channel)
{
	unsigned int value;
	while (1)
	{
		/* wait if mailbox is empty */
		while (mailbox[MAIL0_STATUS]&MAIL_STATUS_EMPTY);
		/* get value@channel */
		value = mailbox[MAIL0_BASE];
		/* check if the expected channel */
		if ((value&MAIL_CHANNEL_MASK)==channel) break;
	}
	return (value&~MAIL_CHANNEL_MASK);
}
/*----------------------------------------------------------------------------*/
void mailbox_write(unsigned int channel,unsigned int value)
{
	/* merge value/channel data */
	value &= ~MAIL_CHANNEL_MASK;
	value |= (channel&MAIL_CHANNEL_MASK);
	/* wait if mailbox is full */
	while (mailbox[MAIL0_STATUS]&MAIL_STATUS_FULL); /* not MAIL1_STAT? */
	/* send it! */
	mailbox[MAIL0_WRITE] = value;
}
/*----------------------------------------------------------------------------*/
