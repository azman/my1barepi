/*----------------------------------------------------------------------------*/
#ifndef __MY1MAILBOXH__
#define __MY1MAILBOXH__
/*----------------------------------------------------------------------------*/
#define MAIL_CHANNEL_MASK 0x0000000F
#define MAIL_CH_POWER 0x00000000
#define MAIL_CH_FBUFF 0x00000001
#define MAIL_CH_VUART 0x00000002
#define MAIL_CH_VCHIQ 0x00000003
#define MAIL_CH_LEDS  0x00000004
#define MAIL_CH_BUTTS 0x00000005
#define MAIL_CH_TOUCH 0x00000006
#define MAIL_CH_NOUSE 0x00000007
#define MAIL_CH_TAGAV 0x00000008
#define MAIL_CH_TAGVA 0x00000009
/*----------------------------------------------------------------------------*/
/**
0: Power management
1: Framebuffer
2: Virtual UART
3: VCHIQ
4: LEDs
5: Buttons
6: Touch screen
7: <NOT USED?>
8: Property tags (ARM -> VC)
9: Property tags (VC -> ARM)
**/
/*----------------------------------------------------------------------------*/
void mailbox_init(void);
unsigned int mailbox_read(unsigned int channel);
void mailbox_write(unsigned int channel,unsigned int value);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
