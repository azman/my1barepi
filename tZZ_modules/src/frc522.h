/*----------------------------------------------------------------------------*/
#ifndef __MY1FRC522_H__
#define __MY1FRC522_H__
/*----------------------------------------------------------------------------*/
#define MAX_SIZE 16
/*----------------------------------------------------------------------------*/
#define FRC522_OK 0
#define FRC522_ERROR_GENERAL 1
#define FRC522_ERROR_NO_TAG 2
/*----------------------------------------------------------------------------*/
/** frc522 commands */
#define FRC522_IDLE         0x00
#define FRC522_MEM25BUFF    0x01
#define FRC522_GENERATE_ID  0x02
#define FRC522_CALC_CRC     0x03
#define FRC522_TRANSMIT     0x04
#define FRC522_NO_CMD_CHG   0x07
#define FRC522_RECEIVE      0x08
#define FRC522_TRANSCEIVE   0x0C
#define FRC522_RESERVED     0x0D
#define FRC522_MF_AUTHENT   0x0E
#define FRC522_SOFT_RESET   0x0F
/*----------------------------------------------------------------------------*/
/** mifare 1 tag commands */
#define MF1_REQIDL    0x26 /* find the antenna area */
#define MF1_REQALL    0x52 /* find all the tags antenna area */
#define MF1_ANTICOLL  0x93 /* anti-collision */
#define MF1_SELECTTAG 0x93 /* election tag */
#define MF1_AUTHENT1A 0x60 /* authentication key A */
#define MF1_AUTHENT1B 0x61 /* authentication key B */
#define MF1_READ      0x30 /* read Block */
#define MF1_WRITE     0xA0 /* write block */
#define MF1_DECREMENT 0xC0 /* debit */
#define MF1_INCREMENT 0xC1 /* recharge */
#define MF1_RESTORE   0xC2 /* transfer block data to the buffer */
#define MF1_TRANSFER  0xB0 /* save the data in the buffer */
#define MF1_HALT      0x50 /* sleep */
/*----------------------------------------------------------------------------*/
/** page 0 regs: command & status */
#define FRC522_P0_RESERVED0 0x00
#define FRC522_P0_COMMAND_REG 0x01
#define FRC522_P0_COMIEN_REG 0x02
#define FRC522_P0_DIVIEN_REG 0x03
#define FRC522_P0_COMIRQ_REG 0x04
#define FRC522_P0_DIVIRQ_REG 0x05
#define FRC522_P0_ERROR_REG 0x06
#define FRC522_P0_STATUS1_REG 0x07
#define FRC522_P0_STATUS2_REG 0x08
#define FRC522_P0_FIFODATA_REG 0x09
#define FRC522_P0_FIFOLVL_REG 0x0A
#define FRC522_P0_FLOWLVL_REG 0x0B
#define FRC522_P0_CONTROL_REG 0x0C
#define FRC522_P0_BITFRAME_REG 0x0D
#define FRC522_P0_COLL_REG 0x0E
#define FRC522_P0_RESERVED1 0x0F
/** page 1 regs: command */
#define FRC522_P1_RESERVED0 0x10
#define FRC522_P1_MODE_REG 0x11
#define FRC522_P1_TX_MODE_REG 0x12
#define FRC522_P1_RX_MODE_REG 0x13
#define FRC522_P1_TX_CNTL_REG 0x14
#define FRC522_P1_TX_ASK_REG 0x15
#define FRC522_P1_TX_SEL_REG 0x16
#define FRC522_P1_RX_SEL_REG 0x17
#define FRC522_P1_RX_THRESH_REG 0x18
#define FRC522_P1_DEMOD_REG 0x19
#define FRC522_P1_RESERVED1 0x1A
#define FRC522_P1_RESERVED2 0x1B
#define FRC522_P1_MF_TX_REG 0x1C
#define FRC522_P1_MF_RX_REG 0x1D
#define FRC522_P1_RESERVED3 0x1E
#define FRC522_P1_UARTSPEED_REG 0x1F
/** page 2 regs: config */
#define FRC522_P2_RESERVED0 0x20
#define FRC522_P2_CRCRESH_REG 0x21
#define FRC522_P2_CRCRESL_REG 0x22
#define FRC522_P2_RESERVED1 0x23
#define FRC522_P2_MODWIDTH_REG 0x24
#define FRC522_P2_RESERVED2 0x25
#define FRC522_P2_RFCFG_REG 0x26
#define FRC522_P2_GSN_REG 0x27
#define FRC522_P2_CWGS_REG 0x28
#define FRC522_P2_MODGS_REG 0x29
#define FRC522_P2_TMODE_REG 0x2A
#define FRC522_P2_TPRESCALER_REG 0x2B
#define FRC522_P2_TRELOADH_REG 0x2C
#define FRC522_P2_TRELOADL_REG 0x2D
#define FRC522_P2_TCOUNTH_REG 0x2E
#define FRC522_P2_TCOUNTL_REG 0x2F
/** page 3 regs: test regs */
#define FRC522_P3_RESERVED0 0x30
#define FRC522_P3_TESTSEL1_REG 0x31
#define FRC522_P3_TESTSEL2_REG 0x32
#define FRC522_P3_TESTPIN_EN_REG 0x33
#define FRC522_P3_TESTPIN_VAL_REG 0x34
#define FRC522_P3_TESTBUS_REG 0x35
#define FRC522_P3_AUTOTEST_REG 0x36
#define FRC522_P3_VERSION_REG 0x37
#define FRC522_P3_ANALOGTEST_REG 0x38
#define FRC522_P3_TESTDAC1_REG 0x39
#define FRC522_P3_TESTDAC2_REG 0x3A
#define FRC522_P3_TESTADC_REG 0x3B
#define FRC522_P3_RESERVED1 0x3C
#define FRC522_P3_RESERVED2 0x3D
#define FRC522_P3_RESERVED3 0x3E
#define FRC522_P3_RESERVED4 0x3F
/*----------------------------------------------------------------------------*/
void frc522_reg_write(int addr,int value);
int frc522_reg_read(int addr);
/*----------------------------------------------------------------------------*/
void frc522_init(void);
void frc522_reset(void);
void frc522_bitmask_set(int addr,int mask);
void frc522_bitmask_clr(int addr,int mask);
int frc522_get_firmware_version(void);
int frc522_digital_self_test(void);
int frc522_cmdtag(int command, unsigned char *data, int dlen,
	unsigned char *resd, int *rlen);
int frc522_reqtag(int mode, unsigned char *type);
int frc522_anti_collision(unsigned char *serial);
void frc522_calculate_crc(unsigned char *data, int dlen, unsigned char *resd);
int frc522_select_tag(unsigned char *serial);
int frc522_authenticate(int mode, int block, unsigned char *key,
	unsigned char *serial);
int frc522_tag_read(int block, unsigned char *data);
int frc522_tag_write(int block, unsigned char *data);
int frc522_halt_tag(void);
/*----------------------------------------------------------------------------*/
/* make a wrapper module? or, publish only functions like this? */
int frc522_get_card_id(unsigned char *id);
/*----------------------------------------------------------------------------*/
#endif /* __MY1FRC522_H__ */
/*----------------------------------------------------------------------------*/
