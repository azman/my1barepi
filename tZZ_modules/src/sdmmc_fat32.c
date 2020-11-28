/*----------------------------------------------------------------------------*/
#include "sdmmc_fat32.h"
/*----------------------------------------------------------------------------*/
extern my1fat32_t part;
/*----------------------------------------------------------------------------*/
int sector_read(word32_t offs)
{
	part.sect.offs = offs;
	offs *= SECTOR_SIZE;
	part.sect.fill = sdmmc_readblock(offs,part.sect.data);
	if (part.sect.fill) part.sect.fill <<= 2;
	return part.sect.fill;
}
/*----------------------------------------------------------------------------*/
int sector_write(void)
{
	word32_t offs = part.sect.offs*SECTOR_SIZE;
	offs = sdmmc_writeblock(offs,part.sect.data);
	if (offs) offs = part.sect.fill;
	return (int) offs;
}
/*----------------------------------------------------------------------------*/
