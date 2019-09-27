/*----------------------------------------------------------------------------*/
#include "frc522.h"
#include "timer.h"
#include "spi.h"
/*----------------------------------------------------------------------------*/
#define FRC522_ADDRESS_MASK 0x7E
#define FRC522_READ_MASK 0x80
/*----------------------------------------------------------------------------*/
void frc522_reg_write(int addr,int value)
{
	spi_activate(SPI_ACTIVATE);
	spi_transfer((addr<<1)&FRC522_ADDRESS_MASK);
	spi_transfer(value);
	spi_activate(SPI_DEACTIVATE);
}
/*----------------------------------------------------------------------------*/
int frc522_reg_read(int addr)
{
	int data;
	spi_activate(SPI_ACTIVATE);
	spi_transfer(((addr<<1)&FRC522_ADDRESS_MASK)|FRC522_READ_MASK);
	data = (int) spi_transfer(0x00);
	spi_activate(SPI_DEACTIVATE);
	return data;
}
/*----------------------------------------------------------------------------*/
#define TIMER_AUTO 0x80
#define TIMER_MASK_PRESCALERH 0x0F
#define TIMER_6780KHZ_PRESCALEH 0x0D
#define TIMER_6780KHZ_PRESCALEL 0x3E
#define MODULATE_FORCE100ASK 0x40
#define MODE_TX_WAITRF 0x20
#define MODE_RESERVED0 0x10
#define MODE_POLMFIN 0x08
#define MODE_RESERVED1 0x04
#define MODE_CRC_PRESET_FFFF 0x03
#define MODE_CRC_PRESET_A671 0x02
#define MODE_CRC_PRESET_6363 0x01
#define MODE_CRC_PRESET_0000 0x00
#define CNTL_TX2RF_ENABLE 0x02
#define CNTL_TX1RF_ENABLE 0x01
/*----------------------------------------------------------------------------*/
void frc522_init(void)
{
	int data = TIMER_AUTO | (TIMER_6780KHZ_PRESCALEH&TIMER_MASK_PRESCALERH);
	/* spi should already be initialized */
	frc522_reset();
	/* for f=6.78MHz? */
	frc522_reg_write(FRC522_P2_TMODE_REG,data);
	frc522_reg_write(FRC522_P2_TPRESCALER_REG,TIMER_6780KHZ_PRESCALEL);
	frc522_reg_write(FRC522_P2_TRELOADL_REG,30);
	frc522_reg_write(FRC522_P2_TRELOADH_REG,0);
	/* config */
	frc522_reg_write(FRC522_P1_TX_ASK_REG,MODULATE_FORCE100ASK);
	data = MODE_TX_WAITRF | MODE_RESERVED0 | MODE_POLMFIN | MODE_RESERVED1 |
		MODE_CRC_PRESET_6363;
	frc522_reg_write(FRC522_P1_MODE_REG,data);
	/* switch on antenna? */
	data = CNTL_TX2RF_ENABLE | CNTL_TX1RF_ENABLE;
	frc522_bitmask_set(FRC522_P1_TX_CNTL_REG,data);
}
/*----------------------------------------------------------------------------*/
void frc522_reset(void)
{
	frc522_reg_write(FRC522_P0_COMMAND_REG,FRC522_SOFT_RESET);
}
/*----------------------------------------------------------------------------*/
void frc522_bitmask_set(int addr,int mask)
{
	int temp = frc522_reg_read(addr);
	frc522_reg_write(addr,temp|mask);
}
/*----------------------------------------------------------------------------*/
void frc522_bitmask_clr(int addr,int mask)
{
	int temp = frc522_reg_read(addr);
	frc522_reg_write(addr,temp&~mask);
}
/*----------------------------------------------------------------------------*/
int frc522_get_firmware_version(void)
{
	return frc522_reg_read(FRC522_P3_VERSION_REG);
}
/*----------------------------------------------------------------------------*/
#define SELFTEST_DATASIZE 64
/*----------------------------------------------------------------------------*/
int frc522_digital_self_test(void)
{
	int loop, test;
	unsigned char self_test_v1[SELFTEST_DATASIZE] =
	{
		0x00, 0xC6, 0x37, 0xD5, 0x32, 0xB7, 0x57, 0x5C,
		0xC2, 0xD8, 0x7C, 0x4D, 0xD9, 0x70, 0xC7, 0x73,
		0x10, 0xE6, 0xD2, 0xAA, 0x5E, 0xA1, 0x3E, 0x5A,
		0x14, 0xAF, 0x30, 0x61, 0xC9, 0x70, 0xDB, 0x2E,
		0x64, 0x22, 0x72, 0xB5, 0xBD, 0x65, 0xF4, 0xEC,
		0x22, 0xBC, 0xD3, 0x72, 0x35, 0xCD, 0xAA, 0x41,
		0x1F, 0xA7, 0xF3, 0x53, 0x14, 0xDE, 0x7E, 0x02,
		0xD9, 0x0F, 0xB5, 0x5E, 0x25, 0x1D, 0x29, 0x79
	};
	unsigned char self_test_v2[SELFTEST_DATASIZE] =
	{
		0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95,
		0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
		0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82,
		0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
		0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81,
		0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
		0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D,
		0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F
	};
	unsigned char *self_test;
	/* test based on firmware version */
	switch(frc522_get_firmware_version())
	{
		case 0x91 :
			self_test = self_test_v1;
			break;
		case 0x92 :
			self_test = self_test_v2;
			break;
		default:
			return -1;
	}
	frc522_reset();
	frc522_reg_write(FRC522_P0_FIFODATA_REG,0x00);
	frc522_reg_write(FRC522_P0_COMMAND_REG,FRC522_MEM25BUFF);
	frc522_reg_write(FRC522_P3_AUTOTEST_REG,0x09);
	frc522_reg_write(FRC522_P0_FIFODATA_REG,0x00);
	frc522_reg_write(FRC522_P0_COMMAND_REG,FRC522_CALC_CRC);
	/* wait... */
	loop = 255;
	do
	{
		test = frc522_reg_read(FRC522_P0_DIVIRQ_REG);
		loop--;
	}
	while (loop>0&&!(test&0x04));
	/* verify */
	for (loop=0;loop<SELFTEST_DATASIZE;loop++)
	{
		test = frc522_reg_read(FRC522_P0_FIFODATA_REG);
		if ((test&0xff)!=(int)self_test[loop])
			return (loop+1);
	}
	/* pass! */
	return 0;
}
/*----------------------------------------------------------------------------*/
#define COMIEN_IRQINV 0x80
#define COMIRQ_SET1 0x80
#define COMIRQ_TIMERIRQ 0x01
#define FIFO_FLUSHBUFF 0x80
#define BITFRAME_STARTSEND 0x80
#define BITFRAME_TXLASTBITS 0x07
#define ERROR_BUFFER_OVERFLOW 0x10
#define ERROR_BIT_COLLISION 0x08
#define ERROR_CRC_FAILED 0x04
#define ERROR_PARITY_FAILED 0x02
#define ERROR_PROTOCOL 0x01
/*----------------------------------------------------------------------------*/
int frc522_cmdtag(int command, unsigned char *data, int dlen,
	unsigned char *resd, int *rlen)
{
	int flag = FRC522_ERROR_GENERAL;
	int irq_enable = 0x00;
	int irq_wait = 0x00;
	int last_bits, temp, loop, test;
	/* check command for wait status */
	switch (command)
	{
		case FRC522_MF_AUTHENT:
			irq_enable = 0x12;
			irq_wait = 0x10;
			break;
		case FRC522_TRANSCEIVE:
			irq_enable = 0x77;
			irq_wait = 0x30;
			break;
	}
	/* prepare */
	frc522_reg_write(FRC522_P0_COMIEN_REG,irq_enable|COMIEN_IRQINV);
	frc522_bitmask_clr(FRC522_P0_COMIRQ_REG,COMIRQ_SET1);
	frc522_bitmask_set(FRC522_P0_FIFOLVL_REG,FIFO_FLUSHBUFF);
	/* idle - cancel current command? */
	frc522_reg_write(FRC522_P0_COMMAND_REG,FRC522_IDLE);
	/* fill tx fifo */
	for (loop=0;loop< dlen;loop++)
		frc522_reg_write(FRC522_P0_FIFODATA_REG,data[loop]);
	/* exec */
	frc522_reg_write(FRC522_P0_COMMAND_REG,command);
	if (command==FRC522_TRANSCEIVE)
		frc522_bitmask_set(FRC522_P0_BITFRAME_REG,BITFRAME_STARTSEND);
	/* wait... <25ms? */
	loop = 25;
	do
	{
		timer_wait(1000);
		temp = frc522_reg_read(FRC522_P0_COMIRQ_REG);
		loop--;
	}
	while ((loop>0)&&!(temp&COMIRQ_TIMERIRQ)&&!(temp&irq_wait));
	frc522_bitmask_clr(FRC522_P0_BITFRAME_REG,BITFRAME_STARTSEND);
	/* check timeout */
	if (loop>0)
	{
		test = ERROR_BUFFER_OVERFLOW | ERROR_BIT_COLLISION |
			ERROR_PARITY_FAILED | ERROR_PROTOCOL; /* ERROR_CRC_FAILED? */
		if(!(frc522_reg_read(FRC522_P0_ERROR_REG)&test))
		{
			flag = FRC522_OK;
			if (temp&irq_enable&0x01)
				flag = FRC522_ERROR_NO_TAG;
			if (command==FRC522_TRANSCEIVE)
			{
				temp = frc522_reg_read(FRC522_P0_FIFOLVL_REG);
				last_bits = frc522_reg_read(FRC522_P0_CONTROL_REG)&0x07;
				if (last_bits)
				{
					*rlen = (temp-1)*8 + last_bits;
				}
				else
				{
					*rlen = temp*8;
				}
				if (temp==0)
					temp = 1;
				if (temp > MAX_SIZE)
					temp = MAX_SIZE;
				for (loop=0;loop<temp;loop++)
					resd[loop] = frc522_reg_read(FRC522_P0_FIFODATA_REG);
			}
		}
		else flag = FRC522_ERROR_GENERAL;
	}
	return flag;
}
/*----------------------------------------------------------------------------*/
#define  MI_TYPE_ULTRALIGHT 0x4400
#define  MI_TYPE_ONE_S50    0x0400
#define  MI_TYPE_ONE_S70    0x0200
#define  MI_TYPE_PRO_X      0x0800
#define  MI_TYPE_DES_FIRE   0x4403
/*----------------------------------------------------------------------------*/
int frc522_reqtag(int mode, unsigned char *type)
{
	int flag, rlen;
	frc522_reg_write(FRC522_P0_BITFRAME_REG,BITFRAME_TXLASTBITS);
	type[0] = mode;
	flag = frc522_cmdtag(FRC522_TRANSCEIVE,type,1,type,&rlen);
	if ((flag != FRC522_OK)||(rlen!=0x10))
		flag = FRC522_ERROR_GENERAL;
	return flag;
}
/*----------------------------------------------------------------------------*/
int frc522_anti_collision(unsigned char *serial)
{
	int flag, loop, rlen;
	unsigned char test = 0x00;

	frc522_reg_write(FRC522_P0_BITFRAME_REG,0x00);
	serial[0] = MF1_ANTICOLL;
	serial[1] = 0x20;
	flag = frc522_cmdtag(FRC522_TRANSCEIVE,serial,2,serial,&rlen);
	rlen = rlen / 8; /* byte counts */
	if (flag==FRC522_OK)
	{
		/* calc checksum */
		for (loop=0;loop<rlen-1;loop++)
			test^=serial[loop];
		/* verify */
		if (test != serial[loop])
			flag = FRC522_ERROR_GENERAL;
	}
	return flag;
}
/*----------------------------------------------------------------------------*/
#define DIVIRQ_CRCIRQ 0x04
/*----------------------------------------------------------------------------*/
void frc522_calculate_crc(unsigned char *data, int dlen, unsigned char *resd)
{
	int loop, temp;
	frc522_bitmask_clr(FRC522_P0_DIVIRQ_REG,DIVIRQ_CRCIRQ);
	frc522_bitmask_set(FRC522_P0_FIFOLVL_REG,FIFO_FLUSHBUFF);
	for (loop=0;loop<dlen;loop++)
		frc522_reg_write(FRC522_P0_FIFODATA_REG,data[loop]);
	frc522_reg_write(FRC522_P0_COMMAND_REG,FRC522_CALC_CRC);
	loop = 0xff;
	do
	{
		temp = frc522_reg_read(FRC522_P0_DIVIRQ_REG);
		loop--;
	}
	while ((loop>0)&&!(temp&DIVIRQ_CRCIRQ));
	/* get results */
	resd[0] = frc522_reg_read(FRC522_P2_CRCRESL_REG);
	resd[1] = frc522_reg_read(FRC522_P2_CRCRESH_REG);
}
/*----------------------------------------------------------------------------*/
int frc522_select_tag(unsigned char *serial)
{
	int loop, flag, rlen, temp;
	unsigned char buff[9];
	buff[0] = MF1_SELECTTAG;
	buff[1] = 0x70;
	for (loop=0;loop<5;loop++)
		buff[loop+2] = serial[loop];
	frc522_calculate_crc(buff,7,&buff[7]);
	flag = frc522_cmdtag(FRC522_TRANSCEIVE,buff,9,buff,&rlen);
	if ((flag==FRC522_OK)&&(rlen==0x18))
		temp = buff[0];
	else
		temp = 0;
	return temp;
}
/*----------------------------------------------------------------------------*/
int frc522_authenticate(int mode, int block, unsigned char *key,
	unsigned char *serial)
{
	int loop, flag, rlen;
	unsigned char buff[12];
	buff[0] = mode;
	buff[1] = block;
	for (loop=0;loop<6;loop++)
		buff[loop+2] = key[loop];
	for (loop=0;loop<4;loop++)
		buff[loop+8] = serial[loop];
	flag = frc522_cmdtag(FRC522_MF_AUTHENT,buff,12,buff,&rlen);
	if ((flag!=FRC522_OK)||(!(frc522_reg_read(FRC522_P0_STATUS2_REG)&0x08)))
		flag = FRC522_ERROR_GENERAL;
	return flag;
}
/*----------------------------------------------------------------------------*/
int frc522_tag_read(int block, unsigned char *data)
{
	int flag, rlen;
	data[0] = MF1_READ;
	data[1] = block;
	frc522_calculate_crc(data,2,&data[2]);
	flag = frc522_cmdtag(FRC522_TRANSCEIVE,data,4,data,&rlen);
	if ((flag!=FRC522_OK)||(rlen!=0x90))
		flag = FRC522_ERROR_GENERAL;
	return flag;
}
/*----------------------------------------------------------------------------*/
int frc522_tag_write(int block, unsigned char *data)
{
	int flag, loop, rlen;
	unsigned char buff[18];
	buff[0] = MF1_WRITE;
	buff[1] = block;
	frc522_calculate_crc(data,2,&data[2]);
	flag = frc522_cmdtag(FRC522_TRANSCEIVE,data,4,data,&rlen);
	if ((flag!=FRC522_OK)||(rlen!=4)||((buff[0]&0x0F)!=0x0A))
		flag = FRC522_ERROR_GENERAL;
	if (flag==FRC522_OK)
	{
		for (loop=0;loop<16;loop++)
			buff[loop] = data[loop];
		frc522_calculate_crc(buff,16,&buff[16]);
		flag = frc522_cmdtag(FRC522_TRANSCEIVE,buff,18,buff,&rlen);
		if ((flag!=FRC522_OK)||(rlen!=4)||((buff[0]&0x0F)!=0x0A))
			flag = FRC522_ERROR_GENERAL;
	}
	return flag;
}
/*----------------------------------------------------------------------------*/
#define MFCRYPTO_ON 0x08
/*----------------------------------------------------------------------------*/
int frc522_halt_tag(void)
{
	int flag, rlen;
	unsigned char buff[4];
	buff[0] = MF1_HALT;
	buff[1] = 0;
	frc522_calculate_crc(buff,2,&buff[2]);
	frc522_bitmask_clr(FRC522_P0_STATUS2_REG,MFCRYPTO_ON);
	flag = frc522_cmdtag(FRC522_TRANSCEIVE,buff,4,buff,&rlen);
	return flag;
}
/*----------------------------------------------------------------------------*/
int frc522_get_card_id(unsigned char *id)
{
	int flag = frc522_reqtag(MF1_REQIDL,id);
	if (flag) return flag;
	return frc522_anti_collision(id);
}
/*----------------------------------------------------------------------------*/
