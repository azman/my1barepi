/*----------------------------------------------------------------------------*/
#ifndef __MY1BOOTH__
#define __MY1BOOTH__
/*----------------------------------------------------------------------------*/
#define SYS0FLAG_ADDR 0x40
#define SYS1FLAG_ADDR 0x44
#define SYS2FLAG_ADDR 0x48
#define SYS3FLAG_ADDR 0x4C
#define SYS4FLAG_ADDR 0x50
#define SYS5FLAG_ADDR 0x54
#define SYS6FLAG_ADDR 0x58
#define SYS7FLAG_ADDR 0x5C
/*----------------------------------------------------------------------------*/
/** 0x544F4F42 => "BOOT" as uint32! */
#define BOOTLOADER_ID 0x544F4F42
/*----------------------------------------------------------------------------*/
unsigned int get32(unsigned int addr);
void put32(unsigned int addr,unsigned int data);
void loopd(unsigned int wait);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
