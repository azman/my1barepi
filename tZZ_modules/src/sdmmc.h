/*----------------------------------------------------------------------------*/
#ifndef __MY1SDMMCH__
#define __MY1SDMMCH__
/*----------------------------------------------------------------------------*/
/** sd card access using emmc device on bcm2835 **/
/*----------------------------------------------------------------------------*/
#include "raspi.h"
#include "boot.h"
/*----------------------------------------------------------------------------*/
#define SDMMC_OK 0
#define SDMMC_ERROR (~(~0UL>>1))
#define SDMMC_TIMEOUT 0x01
/*----------------------------------------------------------------------------*/
/* low speed (400kHz) and high speed (25MHz) */
#define SDMMC_CLK_FREQ_LO 400000
#define SDMMC_CLK_FREQ_HI 25000000
/*----------------------------------------------------------------------------*/
/**  SD base clock is 41.66667Mhz */
#define SDMMC_CLK_BASE_FREQ 41666667
/*----------------------------------------------------------------------------*/
#define SDMMC_RESET_TIMEOUT 100000
#define SDMMC_STABLE_TIMEOUT 100000
#define SDMMC_STATUS_TIMEOUT 500000
#define SDMMC_WAITIN_TIMEOUT 1000000
#define SDMMC_ACMD41_TIMEOUT 10000
#define SDMMC_WAIT_DELAY 1
/*----------------------------------------------------------------------------*/
/* high-capacity flag */
#define SDMMC_FLAG_SDHC 0x01
/*----------------------------------------------------------------------------*/
typedef struct _sdmmc_t
{
	unsigned int stat, hver;
	unsigned int flag, step;
	/* DEBUG: last cmd and arg */
	unsigned int cmd0, arg1;
	/* card id (128bits?) & relative-card-address@rca */
	unsigned int uuid[4], addr;
}
sdmmc_t;
/*----------------------------------------------------------------------------*/
sdmmc_t* get_keep(void);
/*----------------------------------------------------------------------------*/
int sdmmc_init(void);
int sdmmc_readblock(unsigned int lba, unsigned char *buf);
int sdmmc_writeblock(unsigned int lba, unsigned char *buf);
/*----------------------------------------------------------------------------*/
#endif
/*----------------------------------------------------------------------------*/
