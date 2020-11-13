/*----------------------------------------------------------------------------*/
#include "sdmmc.h"
#include "gpio.h"
#include "timer.h"
/*----------------------------------------------------------------------------*/
#define EMMC_BASE (PMAP_BASE|EMMC_OFFSET)
/*----------------------------------------------------------------------------*/
#define EMMC_ARG2        (EMMC_BASE+0x00)
#define EMMC_BLKSIZECNT  (EMMC_BASE+0x04)
#define EMMC_ARG1        (EMMC_BASE+0x08)
#define EMMC_CMDTM       (EMMC_BASE+0x0C)
#define EMMC_RESP0       (EMMC_BASE+0x10)
#define EMMC_RESP1       (EMMC_BASE+0x14)
#define EMMC_RESP2       (EMMC_BASE+0x18)
#define EMMC_RESP3       (EMMC_BASE+0x1C)
#define EMMC_DATA        (EMMC_BASE+0x20)
#define EMMC_STATUS      (EMMC_BASE+0x24)
#define EMMC_CONTROL0    (EMMC_BASE+0x28)
#define EMMC_CONTROL1    (EMMC_BASE+0x2C)
#define EMMC_INTERRUPT   (EMMC_BASE+0x30)
#define EMMC_IRPT_MASK   (EMMC_BASE+0x34)
#define EMMC_IRPT_EN     (EMMC_BASE+0x38)
#define EMMC_CONTROL2    (EMMC_BASE+0x3C)
#define EMMC_SPIINT_SPT  (EMMC_BASE+0xF0)
#define EMMC_SLOTISR_VER (EMMC_BASE+0xFC)
/*----------------------------------------------------------------------------*/
/** EMMC_STATUS bits */
#define EMMC_READ_AVAILABLE 0x0800
#define EMMC_APP_COMMAND 0x20
#define EMMC_DAT_INHIBIT 0x02
#define EMMC_CMD_INHIBIT 0x01
/*----------------------------------------------------------------------------*/
/** EMMC_CONTROL0 bits */
#define EMMC_SPI_MODE_EN  0x00100000
#define EMMC_HCTL_HS_EN   0x00000004
#define EMMC_HCTL_DWIDTH  0x00000002
/*----------------------------------------------------------------------------*/
/** EMMC_CONTROL1 bits */
#define EMMC_SRST_DATA    0x04000000
#define EMMC_SRST_CMD     0x02000000
#define EMMC_SRST_HC      0x01000000
#define EMMC_DATA_TOUNIT  0x000F0000
#define EMMC_CLK_FREQ8    0x0000FF00
#define EMMC_CLK_FREQ_MS2 0x000000C0
#define EMMC_CLK_GENSEL   0x00000020
#define EMMC_CLK_EN       0x00000004
#define EMMC_CLK_STABLE   0x00000002
#define EMMC_CLK_INTLEN   0x00000001
/*----------------------------------------------------------------------------*/
/** EMMC_INTERRUPT bits */
/** EMMC_IRPT_EN bits */
/** EMMC_IRPT_MASK bits - 1=enable */
#define EMMC_IRPT_ALL  0x017F7137
#define EMMC_ACMD_ERR  0x01000000
#define EMMC_DEND_ERR  0x00400000
#define EMMC_DCRC_ERR  0x00200000
#define EMMC_DTO_ERR   0x00100000
#define EMMC_CBAD_ERR  0x00080000
#define EMMC_CEND_ERR  0x00040000
#define EMMC_CCRC_ERR  0x00020000
#define EMMC_CTO_ERR   0x00010000
#define EMMC_ENDBOOT   0x00004000
#define EMMC_BOOTACK   0x00002000
#define EMMC_RETUNE    0x00001000
#define EMMC_CARD      0x00000100
#define EMMC_READ_RDY  0x00000020
#define EMMC_WRITE_RDY 0x00000010
#define EMMC_BLOCK_GAP 0x00000004
#define EMMC_DATA_DONE 0x00000002
#define EMMC_CMD_DONE  0x00000001
/** only for EMMC_IRPT_EN bits - a logical OR of all other errors? */
#define EMMC_ERR_RO    0x00008000
#define INT_ERROR_MASK 0x017e8000
/*----------------------------------------------------------------------------*/
/** EMMC_SLOTISRVER bits */
#define EMMC_SDVENDOR_MASK 0xff000000
#define EMMC_SDVERSION_MASK 0x00ff0000
#define EMMC_SLOTSTAT_MASK 0x000000ff
/*----------------------------------------------------------------------------*/
/** cmdtm stuffs */
#define CMD_INDEX        0x3F000000
#define CMD_TYPE         0x00C00000
#define CMD_ISDATA       0x00200000
#define CMD_IXCHK_EN     0x00100000
#define CMD_CRCCHK_EN    0x00080000
#define CMD_RSPNS_TYPE   0x00030000
#define TM_MULTI_BLOCK   0x00000020
#define TM_DAT_DIR       0x00000010
#define TM_AUTO_CMD_EN   0x0000000C
#define TM_BLKCNT_EN     0x00000002
#define CMD_TYPE_NORMAL  0x00000000
#define CMD_TYPE_SUSPEND 0x00400000
#define CMD_TYPE_RESUME  0x00800000
#define CMD_TYPE_ABORT   0x00C00000
#define ARG_RESP_NONE    0x00000000
#define ARG_RESP_136B    0x00010000
#define ARG_RESP_48B     0x00020000
#define ARG_RESP_48BB    0x00030000
#define TM_DATA_H2C      0x00000000
#define TM_DATA_C2H      TM_DAT_DIR
#define AUTOCMD_NONE     0x00000000
#define AUTOCMD_CMD12    0x00000004
#define AUTOCMD_CMD23    0x00000008
/*----------------------------------------------------------------------------*/
#define CMD0  0x00000000
#define CMD2  0x02000000
#define CMD3  0x03000000
#define CMD7  0x07000000
#define CMD8  0x08000000
#define CMD17 0x11000000
#define CMD23 0x17000000
#define CMD24 0x18000000
#define CMD41 0x29000000
#define CMD55 0x37000000
#define CMD58 0x3a000000
/*----------------------------------------------------------------------------*/
/* commands */
#define CMD_GO_IDLE         (CMD0)
#define CMD_SEND_IF_COND    (CMD8|ARG_RESP_48B)
/** read OperatingConditionsRegister (OCR) */
#define CMD_READ_OCR        (CMD58|ARG_RESP_48B)
/** marker for application command */
#define CMD_APP_CMD         (CMD55)
#define ACMD_SEND_OP_COND   (CMD41|ARG_RESP_48B)
#define CMD_READ_SINGLE     (CMD17|CMD_ISDATA|ARG_RESP_48B|TM_DATA_C2H)
/** get card id */
#define CMD_ALL_SEND_CID    (CMD2|ARG_RESP_136B)
/** get relative card address */
#define CMD_SEND_REL_ADDR   (CMD3|ARG_RESP_48B)
#define CMD_CARD_SELECT     (CMD7|ARG_RESP_48BB)
#define CMD_SET_BLOCKCNT    (CMD23|ARG_RESP_48B)
#define CMD_WRITE_SINGLE    (CMD24|CMD_ISDATA|ARG_RESP_48B)
/*----------------------------------------------------------------------------*/
#define ARG1_IF_COND 0x000001AA
/*----------------------------------------------------------------------------*/
#define OCR_PWRUP_STATUS 0x80000000
/** card-capacity-status */
#define OCR_CCAP_STATUS 0x40000000
#define OCR_VOLTAGE_LVL 0x00FF8000
#define OCR_LOW_VOLTAGE 0x00000080
/*----------------------------------------------------------------------------*/
#define ACMD41_XPC_BIT 0x01000000
#define ACMD41_ARG (OCR_CCAP_STATUS|OCR_VOLTAGE_LVL|ACMD41_XPC_BIT)
/*----------------------------------------------------------------------------*/
#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif
/*----------------------------------------------------------------------------*/
static sdmmc_t keep;
/*----------------------------------------------------------------------------*/
sdmmc_t* get_keep(void)
{
	return &keep;
}
/*----------------------------------------------------------------------------*/
int sdmmc_status(unsigned int pick)
{
	unsigned int wait = SDMMC_STATUS_TIMEOUT;
	while (wait)
	{
		if (!getbit32(EMMC_STATUS,pick)) break;
		if (getbit32(EMMC_INTERRUPT,INT_ERROR_MASK))
		{
			wait = 0;
			break;
		}
		timer_wait(SDMMC_WAIT_DELAY);
		wait--;
	}
	return !wait?SDMMC_ERROR:SDMMC_OK;
}
/*----------------------------------------------------------------------------*/
int sdmmc_interrupt(unsigned int what)
{
	unsigned int temp, mask = what | INT_ERROR_MASK;
	unsigned int wait = SDMMC_WAITIN_TIMEOUT;
	while (wait)
	{
		if (getbit32(EMMC_INTERRUPT,mask))
			break;
		timer_wait(SDMMC_WAIT_DELAY);
		wait--;
	}
	temp = get32(EMMC_INTERRUPT);
	if (!wait||(temp&EMMC_CTO_ERR)||(temp&EMMC_DTO_ERR))
	{
		put32(EMMC_INTERRUPT,temp);
		return SDMMC_TIMEOUT;
	}
	if (temp&INT_ERROR_MASK)
	{
		put32(EMMC_INTERRUPT,temp);
		return (SDMMC_ERROR|temp);
	}
	put32(EMMC_INTERRUPT,what);
	return SDMMC_OK;
}
/*----------------------------------------------------------------------------*/
int sdmmc_command(unsigned int acmd, unsigned int arg1)
{
	keep.arg1 = arg1;
	keep.cmd0 = acmd;
	if (sdmmc_status(EMMC_CMD_INHIBIT)) return SDMMC_TIMEOUT;
	put32(EMMC_INTERRUPT,get32(EMMC_INTERRUPT));
	put32(EMMC_ARG1,arg1);
	put32(EMMC_CMDTM,acmd);
	return sdmmc_interrupt(EMMC_CMD_DONE);
}
/*----------------------------------------------------------------------------*/
int sdmmc_clkfreq(unsigned int freq)
{
	unsigned int cdiv, temp, next;
	unsigned int wait = SDMMC_STATUS_TIMEOUT;
	while (wait)
	{
		if (!getbit32(EMMC_STATUS,EMMC_DAT_INHIBIT|EMMC_CMD_INHIBIT))
			break;
		timer_wait(SDMMC_WAIT_DELAY);
		wait--;
	}
	if (!wait) return SDMMC_ERROR;
	/* disable clock */
	clrbit32(EMMC_CONTROL1,EMMC_CLK_EN);
	cdiv = SDMMC_CLK_BASE_FREQ/freq;
	if (keep.hver<=2)
	{
		temp = cdiv - 1; next = 32;
		if(!temp) next=0;
		else
		{
			unsigned int test = 0x10000;
			while (temp<test)
			{
				next >>= 1;
				if (!next) break;
				test >>= (next>>1);
			}
			if (next>0) next--;
			if (next>7) next = 7;
		}
		cdiv=(1<<next);
	}
	if (cdiv<2) cdiv = 2;
	/* rearrange 10-bits divisor */
	temp = ((cdiv&0xff)<<8)|((cdiv&0x300)>>2);
	clrbit32(EMMC_CONTROL1,EMMC_CLK_FREQ8|EMMC_CLK_FREQ_MS2);
	setbit32(EMMC_CONTROL1,temp);
	/* enable clock */
	setbit32(EMMC_CONTROL1,EMMC_CLK_EN);
	wait = SDMMC_STABLE_TIMEOUT;
	while (wait)
	{
		if (getbit32(EMMC_CONTROL1,EMMC_CLK_STABLE))
			break;
		timer_wait(SDMMC_WAIT_DELAY);
		wait--;
	}
	if (!wait) return SDMMC_ERROR;
	return SDMMC_OK;
}
/*----------------------------------------------------------------------------*/
int sdmmc_init(void)
{
	unsigned int wait, temp;
	int test;
	/** GPIO_CD=47 */
	gpio_config(47,GPIO_ALTF3);
	gpio_pull(47,GPIO_PULL_UP);
	/** GPIO_CLK=48, GPIO_CMD=49 */
	gpio_config(48,GPIO_ALTF3);
	gpio_config(49,GPIO_ALTF3);
	gpio_pull(48,GPIO_PULL_UP);
	gpio_pull(49,GPIO_PULL_UP);
	/** GPIO_DAT{0-3}=50-53 */
	gpio_config(50,GPIO_ALTF3);
	gpio_config(51,GPIO_ALTF3);
	gpio_config(52,GPIO_ALTF3);
	gpio_config(53,GPIO_ALTF3);
	gpio_pull(50,GPIO_PULL_UP);
	gpio_pull(51,GPIO_PULL_UP);
	gpio_pull(52,GPIO_PULL_UP);
	gpio_pull(53,GPIO_PULL_UP);
	/* host version */
	keep.hver = getbit32(EMMC_SLOTISR_VER,EMMC_SDVERSION_MASK)>>16;
	keep.flag = 0;
	keep.step = 0;
	/* just in case - these are default power-up reset values */
	put32(EMMC_CONTROL0,0);
	put32(EMMC_CONTROL1,0);
	/* reset host circuit */
	setbit32(EMMC_CONTROL1,EMMC_SRST_HC);
	wait = SDMMC_RESET_TIMEOUT;
	while (wait)
	{
		if (!getbit32(EMMC_CONTROL1,EMMC_SRST_HC))
			break;
		timer_wait(SDMMC_WAIT_DELAY);
		wait--;
	}
	if (!wait) return SDMMC_ERROR;
	/* set max timeout and enable internal clock */
	temp = get32(EMMC_CONTROL1);
	temp &= ~EMMC_DATA_TOUNIT;
	temp |= (0xE<<16); /* max timeout, 0xF disables */
	temp |= EMMC_CLK_INTLEN;
	put32(EMMC_CONTROL1,temp);
	/* try to set clock at low speed */
	if ((test=sdmmc_clkfreq(SDMMC_CLK_FREQ_LO)))
		return test;
	/* enable ALL interrupt? */
	put32(EMMC_IRPT_MASK,EMMC_IRPT_ALL);
	put32(EMMC_IRPT_EN,EMMC_IRPT_ALL);
	/* reset/start spi mode */
	test = sdmmc_command(CMD_GO_IDLE,0);
	if (test) return test;
	keep.step++; /** STEP! 1ST! */
	/* check op voltage - MUST get exact response pattern */
	test = sdmmc_command(CMD_SEND_IF_COND,ARG1_IF_COND);
	if (test) return test;
	if (get32(EMMC_RESP0)!=ARG1_IF_COND)
		return SDMMC_ERROR;
	keep.step++; /** STEP! */
	/* set operating condition - 1s max? */
	wait = SDMMC_ACMD41_TIMEOUT;
	while (wait)
	{
		/* duh! required for ACMDxx! no checking required */
		sdmmc_command(CMD_APP_CMD,0);
		timer_wait(100);
		/* set operating condition - hcs bit! */
		test = sdmmc_command(ACMD_SEND_OP_COND,ACMD41_ARG);
		if (test&SDMMC_ERROR) return test;
		if (get32(EMMC_RESP0)&OCR_PWRUP_STATUS)
		{
			if (get32(EMMC_RESP0)&OCR_CCAP_STATUS)
				keep.flag |= SDMMC_FLAG_SDHC;
			break;
		}
		timer_wait(100*SDMMC_WAIT_DELAY);
		wait--;
	}
	keep.step++; /** STEP! => WANNA SEE IF WE GET HERE! */
	if (!wait) return SDMMC_TIMEOUT;
	keep.step++; /** STEP! */
	/* get card id */
	test = sdmmc_command(CMD_ALL_SEND_CID,0);
	if (test) return test;
	keep.uuid[0] = get32(EMMC_RESP0);
	keep.uuid[1] = get32(EMMC_RESP1);
	keep.uuid[2] = get32(EMMC_RESP2);
	keep.uuid[3] = get32(EMMC_RESP3);
	keep.step++; /** STEP! */
	/* get relative card address */
	test = sdmmc_command(CMD_SEND_REL_ADDR,0);
	if (test) return test;
	keep.addr = get32(EMMC_RESP0);
	keep.step++; /** STEP! */
	/* use higher clock freq! */
	test = sdmmc_clkfreq(SDMMC_CLK_FREQ_HI);
	if (test) return test;
	/* card select */
	test = sdmmc_command(CMD_CARD_SELECT,keep.addr);
	if (test) return test;
	if (sdmmc_status(EMMC_DAT_INHIBIT))
		return SDMMC_TIMEOUT;
	return SDMMC_OK;
}
/*----------------------------------------------------------------------------*/
int sdmmc_readblock(unsigned int lba, unsigned char *buffer)
{
	int test, loop;
	unsigned int *buf = (unsigned int*)buffer;
	if (sdmmc_status(EMMC_DAT_INHIBIT))
	{
		keep.stat = SDMMC_TIMEOUT<<1;
		return 0;
	}
	/* just in case */
	put32(EMMC_BLKSIZECNT,(1 << 16)|SECTOR_SIZE);
	/* request data */
	test = sdmmc_command(CMD_READ_SINGLE,lba);
	if (test)
	{
		keep.stat = test;
		return 0;
	}
	test = sdmmc_interrupt(EMMC_READ_RDY);
	if (test)
	{
		keep.stat = test;
		return 0;
	}
	for (loop=0;loop<128;loop++)
		buf[loop] = get32(EMMC_DATA);
	return loop;
}
/*----------------------------------------------------------------------------*/
int sdmmc_writeblock(unsigned int lba, unsigned char *buffer)
{
	int test, loop;
	unsigned int *buf = (unsigned int*)buffer;
	if (sdmmc_status(EMMC_DAT_INHIBIT))
	{
		keep.stat = SDMMC_TIMEOUT<<1;
		return 0;
	}
	/* just in case */
	put32(EMMC_BLKSIZECNT,(1 << 16)|SECTOR_SIZE);
	/* request data */
	test = sdmmc_command(CMD_WRITE_SINGLE,lba);
	if (test)
	{
		keep.stat = test;
		return 0;
	}
	test = sdmmc_interrupt(EMMC_WRITE_RDY);
	if (test)
	{
		keep.stat = test;
		return 0;
	}
	for (loop=0;loop<128;loop++)
		put32(EMMC_DATA,buf[loop]);
	test = sdmmc_interrupt(EMMC_DATA_DONE);
	if (test)
	{
		keep.stat = test;
		return 0;
	}
	return loop;
}
/*----------------------------------------------------------------------------*/
