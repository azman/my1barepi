/*----------------------------------------------------------------------------*/
#include "video.h"
#include "sdmmc_fat32.h"
/*----------------------------------------------------------------------------*/
#define LOG_FILE "DATA.LOG"
#define LOG_DATA "I AM LEGEND!\r\n"
#define LOG_DATA_LEN 14
/*----------------------------------------------------------------------------*/
extern my1fat32_t part;
/*----------------------------------------------------------------------------*/
void sector_show(my1sector_t* sector)
{
	unsigned long offs;
	unsigned long ends;
	int loop, cols, test;
	/* initialize */
	offs = sector->offs * SECTOR_SIZE;
	ends = offs + SECTOR_SIZE;
	loop = 0; cols = 0;
	/* loop through sector data */
	while(loop<SECTOR_SIZE&&offs<ends)
	{
		if(cols==0)
		{
			video_text_hexuint(offs);
			video_text_string(" - ");
		}
		test = sector->data[loop];
		loop++;
		/* data in hex */
		video_text_hexbyte(test);
		video_text_string(" ");
		cols++;
		/* check maximum line buffer */
		if(cols==16)
		{
			video_text_string("\n");
			cols = 0;
			offs += 16;
		}
	}
	video_text_string("\n");
}
/*----------------------------------------------------------------------------*/
void main(void)
{
	fat32_vbr_t save;
	my1file_t file;
	word32_t temp;
	byte08_t *mark;
	int test, loop;
	video_init(VIDEO_RES_MAX); /** or, VIDEO_RES_VGA */
	video_set_bgcolor(COLOR_BLUE);
	video_clear();
	do
	{
		video_text_string("MY1BAREPI FAT32 LIBRARY\n\n");
		test = sdmmc_init();
		video_text_string("EMMC Init=");
		video_text_integer(test);
		video_text_string("\n");
		if (test) break;
		video_text_string("Find Part=");
		temp = fat32_init();
		video_text_hexuint(temp);
		video_text_string("\n");
		if (temp)
		{
			switch (temp)
			{
				case FAT32_MBR_FAILED:
					video_text_string("** Cannot read MBR!\n)");
					break;
				case FAT32_MBR_INVALID:
					video_text_string("** Invalid MBR signature => [");
					video_text_hexbyte(part.sect.data[510]);
					video_text_string("][");
					video_text_hexbyte(part.sect.data[511]);
					video_text_string("]\n");
					break;
				case FAT32_NOT_FOUND:
					video_text_string("** Cannot find FAT32 partition!\n");
					break;
				case FAT32_VBR_FAILED:
					video_text_string("** Cannot read VBR!\n)");
					break;
				case FAT32_VBR_INVALID:
					video_text_string("** Invalid VBR signature => [");
					video_text_hexbyte(part.sect.data[510]);
					video_text_string("][");
					video_text_hexbyte(part.sect.data[511]);
					video_text_string("]\n");
					break;
				default:
					video_text_string("** Unknown error!\n");
			}
			break;
		}
		video_text_string("-- FAT32 partition @ sector ");
		video_text_integer(part.next);
		video_text_string("\n");
		sector_read(part.next);
		mark = (byte08_t*) &save;
		for (loop=0;loop<SECTOR_SIZE;loop++)
			mark[loop] = part.sect.data[loop];
		temp = part.sect.offs;
		fat32_root(); /* reload root dir */
		video_text_string("-- FAT32 Info: (");
		video_text_hexuint(part.flag);
		video_text_string(")\n");
		video_text_string("## Partition offset: ");
		video_text_integer(temp);
		video_text_string("\n");
		video_text_string("## FAT start sector: ");
		video_text_integer(part.ifat);
		video_text_string(" (");
		video_text_integer(part.ifat-temp);
		video_text_string(")\n");
		video_text_string("## DAT start sector: ");
		video_text_integer(part.data);
		video_text_string(" (");
		video_text_integer(part.data-temp);
		video_text_string(")\n");
		video_text_string("## Sector(s) per cluster: ");
		video_text_integer(part._spc);
		video_text_string("\n");
		video_text_string("## Hidden sectors: ");
		video_text_integer(save.meta.hidsec);
		video_text_string("\n");
		video_text_string("## LargeVol sectors: ");
		video_text_integer(save.meta.sctlrg);
		video_text_string("\n");
		video_text_string("## Sector(s) per FAT: ");
		video_text_integer(save.meta.scpfat);
		video_text_string("\n");
		video_text_string("## Mirror Flag: 0x");
		video_text_hexuint(save.meta.m_flag);
		video_text_string("\n");
		video_text_string("## FS version: ");
		video_text_integer(save.meta.fsvers);
		video_text_string("\n");
		video_text_string("## Root cluster: ");
		video_text_integer(save.meta.clroot);
		video_text_string(" (");
		video_text_integer(part.root);
		video_text_string(")\n");
		video_text_string("## FS info sector: ");
		video_text_integer(save.meta.fsinfo);
		video_text_string("\n");
		video_text_string("## Backup boot sector: ");
		video_text_integer(save.meta.backbs);
		video_text_string("\n");
		video_text_string("## DriveNumber: ");
		video_text_integer(save.meta.drvnum);
		video_text_string("\n");
		video_text_string("## ExtSig: 0x");
		video_text_hexbyte(save.meta.extsig);
		video_text_string("\n");
		video_text_string("## SerialNum: 0x");
		video_text_hexuint(save.meta.sernum);
		video_text_string("\n");
		video_text_string("## Volume Label: '");
		for (loop=0;loop<11;loop++)
			video_text_char(save.meta.vollbl[loop]);
		video_text_string("'\n");
		video_text_string("## FS Type: '");
		for (loop=0;loop<8;loop++)
			video_text_char(save.meta.fstype[loop]);
		video_text_string("'\n");
		video_text_string("## {File:" LOG_FILE "} {");
		test = fat32_open(&file,LOG_FILE,FILE_OPTS_APPEND);
		if (!test)
		{
			test = fat32_open(&file,LOG_FILE,FILE_OPTS_APPEND|FILE_OPTS_CREATE);
			if (!test)
			{
				video_text_string("}\n** Failed to create '");
				video_text_string(file.name);
				video_text_string("'!\n");
				break;
			}
			else video_text_string("NEW");
		}
		else
		{
			video_text_string("Clus:");
			video_text_integer(file.curr);
			video_text_string(",Size:");
			video_text_integer(file.size);
		}
		video_text_string("}\n");
		video_text_string("-- Writing data... ");
		test = fat32_write(&file,(byte08_t*)LOG_DATA,LOG_DATA_LEN);
		video_text_string("done. (");
		video_text_hexuint(test);
		video_text_string(")\n");
		if (test!=LOG_DATA_LEN)
		{
			video_text_string("\n@@ DEBUG: ");
			video_text_string("{Offs:");
			video_text_integer(part.sect.offs*512);
			video_text_string(",Sect:");
			video_text_integer(part.sect.offs);
			video_text_string("}\n");
			sector_show(&part.sect);
			video_text_string("\n");
		}
		fat32_close(&file);
		test = fat32_open(&file,LOG_FILE,FILE_OPTS_DOREAD);
		if (!test)
		{
			video_text_string("** Cannot read file '");
			video_text_string(file.name);
			video_text_string("'!\n");
			break;
		}
		video_text_string("## {Read:");
		video_text_string(file.name);
		video_text_string("} {");
		video_text_string("Clus:");
		video_text_integer(file.curr);
		video_text_string(",Size:");
		video_text_integer(file.size);
		video_text_string("}\n");
	}
	while (0);
	/** do initialization */
	while(1)
	{
		/** do your stuff */
	}
}
/*----------------------------------------------------------------------------*/
