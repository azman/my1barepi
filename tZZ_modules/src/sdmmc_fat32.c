/*----------------------------------------------------------------------------*/
#include "sdmmc_fat32.h"
#include "mbr.h"
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
int sector_find_fat32(void)
{
	mbr_t* pmbr;
	int loop, pick;
	/* read first sector - look for partition table */
	if ((loop=sector_read(0))!=SECTOR_SIZE)
		return FFAT32_MBR_FAILED;
	pmbr = (mbr_t*)part.sect.data;
	if (pmbr->ends[0]!=0x55||pmbr->ends[1]!=0xaa)
		return FFAT32_MBR_INVALID;
	/* check all primary partitions */
	for (loop=0,pick=-1;loop<4;loop++)
	{
		if (!pmbr->ptab[loop].sysid) continue;
		if (pmbr->ptab[loop].sysid==PARTID_FAT32||
			pmbr->ptab[loop].sysid==PARTID_FAT32_LBA)
		{
			pick = loop;
			break;
		}
	}
	if (pick<0)
		return FFAT32_NOT_FOUND;
	if ((loop=sector_read(pmbr->ptab[pick].rsect))<=0)
		return FFAT32_VBR_INVALID;
	return 0;
}
/*----------------------------------------------------------------------------*/
