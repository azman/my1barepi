/*----------------------------------------------------------------------------*/
#ifndef __MY1SDEXT_H__
#define __MY1SDEXT_H__
/*----------------------------------------------------------------------------*/
#define SDCARD_CMD00 0x00
#define SDCARD_CMD00_CRC 0x95
#define SDCARD_CMD01 0x01
#define SDCARD_CMD01_CRC 0xF9
#define SDCARD_CMD08 0x08
#define SDCARD_CMD08_ARG 0x000001AA
#define SDCARD_CMD08_CRC 0x87
#define SDCARD_CMD12 0x0C
#define SDCARD_CMD16 0x10
#define SDCARD_CMD17 0x11
#define SDCARD_CMD41 0x29
/** CMD41 arg/crc assumes HCS (high capacity?), else 0x00000000 & 0xE5 */
#define SDCARD_CMD41_ARG 0x40000000
#define SDCARD_CMD41_CRC 0x77
#define SDCARD_CMD55 0x37
#define SDCARD_CMD55_CRC 0x65
#define SDCARD_CMD59 0x3B
/*----------------------------------------------------------------------------*/
#define SDCARD_SWRESET SDCARD_CMD00
#define SDCARD_DOINIT SDCARD_CMD01
#define SDCARD_STOPTX SDCARD_CMD12
#define SDCARD_CHBSIZE SDCARD_CMD16
#define SDCARD_RDBLOCK SDCARD_CMD17
#define SDCARD_APPCMD SDCARD_CMD55
/*----------------------------------------------------------------------------*/
#define SDCARD_ARG_NONE_ 0x00000000
#define SDCARD_DUMMY_DATA 0xFF
#define SDCARD_DUMMY_CRC SDCARD_DUMMY_DATA
/*----------------------------------------------------------------------------*/
#define SDCARD_RESP_SUCCESS 0x00
#define SDCARD_RESP_R1_IDLE 0x01
#define SDCARD_RESP_ILLEGAL 0x05
#define SDCARD_RESP_INVALID 0xFF
#define SDCARD_RESP_MASK 0xFF
/*----------------------------------------------------------------------------*/
#define SDCARD_FLUSH_R1 1
#define SDCARD_FLUSH_R7 4
/*----------------------------------------------------------------------------*/
#define SDCARD_ERROR_FLAG 0x80000000
#define SDCARD_ERROR_CMD55 (SDCARD_ERROR_FLAG|0x00010000)
#define SDCARD_ERROR_ACMD41 (SDCARD_ERROR_FLAG|0x00020000)
/*----------------------------------------------------------------------------*/
#define SDCARD_SECTOR_SIZE 512
/*----------------------------------------------------------------------------*/
void sdext_init(void);
unsigned int sdext_idle(void);
unsigned int sdext_cmd8(unsigned char* buffer);
unsigned int sdext_cmd1(void);
unsigned int sdext_acmd41(void);
unsigned int sdext_disable_crc(void);
unsigned int sdext_blocksize(void);
unsigned int sdext_read_block(unsigned int sector, unsigned char* buffer);
/*----------------------------------------------------------------------------*/
#endif /* __MY1SDEXT_H__ */
/*----------------------------------------------------------------------------*/
