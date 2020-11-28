/*----------------------------------------------------------------------------*/
#include "fat32.h"
/*----------------------------------------------------------------------------*/
/** from my1fat32 project */
/*----------------------------------------------------------------------------*/
my1fat32_t part;
fat32_dir_t* pdir;
char dosname[FAT32_NAME_BUFFSIZE];
/*----------------------------------------------------------------------------*/
/** useful macros! */
#define fat32dir_ispath(pdir) (pdir->attr&FAT32_ATTR_SUB_DIR)?1:0
#define fat32dir_cluster(pdir) (word32_t)(pdir->clhi<<16)+pdir->clus
#define fat32dir_empty(pdir) ((pdir->name[0]==0x00)||(pdir->name[0]==0xe5))
/*----------------------------------------------------------------------------*/
/* first 2 clusters are NOT used! */
#define fat32_cl2sect(pick) (((pick-2)*part._spc)+part.data)
/*----------------------------------------------------------------------------*/
#define FAT32_DIR_MAXCOUNT (SECTOR_SIZE/sizeof(fat32_dir_t))
#define FAT32_DIR_FIRST (fat32_dir_t*)part.sect.data
/*----------------------------------------------------------------------------*/
/* load given cluster */
word32_t fat32_load_that(word32_t that)
{
	if (sector_read(fat32_cl2sect(that))==SECTOR_SIZE)
	{
		part.curr = that;
		part._sec = 1; /* next sector to load */
		return that;
	}
	part.flag |= FAT32_SECTOR_ERROR;
	return FAT32_FAILURE;
}
/*----------------------------------------------------------------------------*/
/* load fat32 information */
word32_t fat32_init(void)
{
	/* assume vbr loaded in part.sect */
	fat32_vbr_t* pvbr = (fat32_vbr_t*)part.sect.data;
	/* setup flag, default opts */
	part.flag = FAT32_FLAG_OK;
	part.opts = 0;
	/* should have boot marker 0xaa55 */
	if (pvbr->ends[0]!=0x55||pvbr->ends[1]!=0xaa)
	{
		part.flag |= FAT32_BOOT_MISSING;
		return FAT32_FAILURE;
	}
	/* 16-bit root entry should be zero for fat32 */
	if (pvbr->meta.biospb.rooten)
	{
		part.flag |= FAT32_INVALID_ENTRY;
		return FAT32_FAILURE;
	}
	/* only 512-byte sectors are supported? */
	if (pvbr->meta.biospb.bpsect!=SECTOR_SIZE)
	{
		part.flag |= FAT32_INVALID_SSIZE;
		return FAT32_FAILURE;
	}
	/* fatst - initial sector + reserved sectors count */
	part.ifat = part.sect.offs + pvbr->meta.biospb.rsccnt;
	/* init sector for data cluster */
	part.data = pvbr->meta.biospb.fatcnt * pvbr->meta.scpfat;
	part.data += part.ifat;
	/* keep sector-per-cluster info */
	part._spc = pvbr->meta.biospb.spclus;
	/* root cluster */
	part.root = pvbr->meta.clroot;
	/* start to read root cluster */
	if (!fat32_load_that(part.root))
	{
		/* FAT32_SECTOR_ERROR! */
		return FAT32_FAILURE;
	}
	/* first cluster in chain */
	part.cls1 = part.curr;
	return part.curr;
}
/*----------------------------------------------------------------------------*/
/* get/set fat cluster table */
word32_t fat32fat_link(word32_t curr, word32_t mark)
{
	word32_t temp, *pnxt;
	do
	{
		temp = curr/128;
		if (sector_read(part.ifat+temp)!=SECTOR_SIZE)
			break;
		temp = curr%128;
		pnxt = (word32_t*) part.sect.data;
		temp = pnxt[temp]; /* should not be zero? because linked! */
		if (mark!=FAT32_VOID)
		{
			pnxt[curr%128] = mark;
			if (sector_write()!=part.sect.fill)
				break;
		}
		return temp;
	}
	while (0);
	part.flag |= FAT32_SECTOR_ERROR;
	return FAT32_FAILURE;
}
/*----------------------------------------------------------------------------*/
/* get first empty cluster from fat */
word32_t fat32_get_empty_cluster(void)
{
	word32_t temp, isec, *ptmp;
	temp = 2; isec = part.ifat;
	while (isec<part.data)
	{
		if (sector_read(isec)!=SECTOR_SIZE)
		{
			part.flag |= FAT32_SECTOR_ERROR;
			return FAT32_FAILURE;
		}
		ptmp = (word32_t*) part.sect.data;
		for (isec=temp==2?temp:0;isec<128;isec++,temp++)
			if (ptmp[isec]==FAT32_FREE_CLUSTER) return temp;
		isec = part.sect.offs+1; /* next sector of fat */
	}
	return FAT32_FAILURE;
}
/*----------------------------------------------------------------------------*/
/* get next cluster (or sector, if multiple sector per cluster) */
word32_t fat32_next(void)
{
	if (part._sec<part._spc)
	{
		if (sector_read(part.sect.offs+1)==SECTOR_SIZE)
		{
			part._sec++;
			return part.curr; /* still in this cluster */
		}
		part.flag |= FAT32_SECTOR_ERROR;
	}
	else
	{
		part.next = fat32fat_link(part.curr,FAT32_VOID);
		if (!part.next||part.next==FAT32_LAST_CLUSTER)
			return part.next;
		/* do not update part.cls1 here! */
		return fat32_load_that(part.next);
	}
	return FAT32_FAILURE;
}
/*----------------------------------------------------------------------------*/
/* fills dosname c string from pdir 8+3 name entry */
void fat32dir_name(void)
{
	/* get 8+3 dos name from fat32_dir_t */
	int loop, test;
	for (loop=0;loop<8;loop++)
	{
		if (pdir->name[loop]==0x20)
		{
			dosname[loop] = 0x0;
			break;
		}
		else dosname[loop] = pdir->name[loop];
	}
	test = loop;
	if (pdir->name[8]&&pdir->name[8]!=0x20)
	{
		dosname[test++] = '.';
		dosname[test++] = pdir->name[8];
		for (loop=9;loop<11;loop++,test++)
		{
			if (!pdir->name[loop]||pdir->name[loop]==0x20)
			{
				dosname[test] = 0x0;
				break;
			}
			else dosname[test] = pdir->name[loop];
		}
	}
	dosname[test] = 0x0;
}
/*----------------------------------------------------------------------------*/
/* compare dosname with given name, returns 1 if the same */
int fat32dir_name_cmp(char* name)
{
	int loop;
	for (loop=0;loop<FAT32_NAME_BUFFSIZE;loop++)
	{
		if (name[loop]!=dosname[loop]) break;
		if (!name[loop]) return 1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
/* finds fat32_dir_t entry with given name */
fat32_dir_t* fat32_find_name(char* name)
{
	/* find 'name' in current path */
	int loop;
	word32_t temp = fat32_load_that(part.cls1);
	while (temp!=FAT32_FAILURE&&temp!=FAT32_LAST_CLUSTER)
	{
		pdir = FAT32_DIR_FIRST;
		for (loop=0;loop<FAT32_DIR_MAXCOUNT;loop++,pdir++)
		{
			if (pdir->name[0]==0x00) return 0x0;
			/* deleted item */
			if (pdir->name[0]==0xe5) continue;
			/* check 8+3 name */
			fat32dir_name();
			if (fat32dir_name_cmp(name))
				return pdir;
		}
		temp = fat32_next();
	}
	return 0x0;
}
/*----------------------------------------------------------------------------*/
/* load first data cluster for fat32_find_name output */
word32_t fat32_load_name(char* name)
{
	if (!fat32_find_name(name)) return FAT32_FAILURE;
	if (!fat32_load_that(fat32dir_cluster(pdir))) return FAT32_FAILURE;
	/* first cluster in chain */
	part.cls1 = part.curr;
	return part.curr;
}
/*----------------------------------------------------------------------------*/
/* finds a free fat32_dir_t entry - expands path cluster as needed */
fat32_dir_t* fat32dir_find_free(void)
{
	int loop;
	word32_t temp = fat32_load_that(part.cls1);
	while (temp!=FAT32_FAILURE)
	{
		pdir = FAT32_DIR_FIRST;
		for (loop=0;loop<FAT32_DIR_MAXCOUNT;loop++,pdir++)
		{
			if (fat32dir_empty(pdir))
				return pdir;
		}
		temp = fat32_next();
		/* should ALWAYS find a free dir entry? */
		if (temp==FAT32_LAST_CLUSTER)
		{
			temp = fat32_get_empty_cluster();
			if (temp==FAT32_FAILURE) break;
			fat32fat_link(part.curr,temp);
			temp = fat32_load_that(temp);
			if (temp==FAT32_FAILURE) break;
			pdir = FAT32_DIR_FIRST;
			for (loop=0;loop<FAT32_DIR_MAXCOUNT;loop++,pdir++)
				pdir->name[0] = 0x0;
			pdir = FAT32_DIR_FIRST;
			return pdir;
		}
	}
	return 0x0;
}
/*----------------------------------------------------------------------------*/
/* fills pdir 8+3 name entry from dosname c string - assumed valid allcaps */
void fat32dir_name_make(char* name)
{
	int loop, temp;
	for (loop=0,temp=0;loop<8;loop++)
	{
		if (!name[temp]||name[temp]=='.')
			pdir->name[loop] = 0x20;
		else
			pdir->name[loop] = name[temp++];
	}
	/* find extension in source */
	while (name[temp])
	{
		if (name[temp]=='.')
			break;
		temp++;
	}
	if (!name[temp]) temp = 0;
	else temp++;
	/* copy in - loop should be 8 here! */
	for (;loop<11;loop++)
	{
		if (!temp) pdir->name[loop] = 0x20;
		else pdir->name[loop] = name[temp++];
	}
}
/*----------------------------------------------------------------------------*/
/* write to a file in current path */
int fat32_write_file(char* name, byte08_t* data, int size)
{
	word32_t temp, next, fill;
	int loop;
	pdir = fat32_find_name(name);
	/* create if not existing! */
	if (!pdir)
	{
		/* find first empy entry - should always find one? */
		pdir = fat32dir_find_free();
		if (!pdir) return (int)FAT32_FIND_FREE|part.flag;
		/* fill in info */
		fat32dir_name_make(name);
		pdir->attr = 0x00; /* basic file */
		pdir->clhi = 0;
		pdir->clus = 0;
		pdir->size = 0;
		/* update that */
		if (sector_write()!=part.sect.fill)
			return (int)FAT32_DIR_WRITE|part.flag;
		if (size>0)
		{
			/* find first empy cluster */
			temp = fat32_get_empty_cluster();
			if (!temp) return (int)FAT32_FULL_CLUSTER;
			/* mark as used! */
			fat32fat_link(temp,FAT32_LAST_CLUSTER);
			/* get back that entry... should be available now! */
			pdir = fat32_find_name(name);
			if (!pdir) return (int)FAT32_FLAG_ERROR;
			/* update cluster info */
			pdir->clhi = (temp>>16)&0xffff;
			pdir->clus = temp&0xffff;
			/* update that */
			if (sector_write()!=part.sect.fill)
				return (int)FAT32_SECTOR_ERROR;
		}
		fill = 0;
		loop = 0;
	}
	else
	{
		if (pdir->attr&FAT32_ATTR_SUB_DIR)
			return (int)FAT32_NOT_FILE;
		/* keep current total size AND current sector free size */
		fill = pdir->size;
		loop = pdir->size%SECTOR_SIZE;
		/* go to last cluster */
		temp = fat32dir_cluster(pdir);
		if (!temp) /** probably !pdir->size as well! */
		{
			temp = fat32_get_empty_cluster();
			if (temp==FAT32_FAILURE)
				return (int)FAT32_FLAG_ERROR;
			fat32fat_link(temp,FAT32_LAST_CLUSTER);
			/* update pdir entry */
			pdir = fat32_find_name(name);
			if (!pdir) return (int)FAT32_FLAG_ERROR;
			pdir->clhi = (temp>>16)&0xffff;
			pdir->clus = temp&0xffff;
			if (sector_write()!=part.sect.fill)
				return (int)FAT32_SECTOR_ERROR;
		}
		else
		{
			if (part.opts&FAT32_OPTS_WRITE_OVERWRITE)
			{
				fill = 0;
				loop = 0;
				pdir->size = 0;
				if (sector_write()!=part.sect.fill)
					return (int)FAT32_SECTOR_ERROR;
				next = fat32fat_link(temp,FAT32_VOID);
				/* release all clusters */
				while (next!=FAT32_LAST_CLUSTER)
					next = fat32fat_link(next,FAT32_FREE_CLUSTER);
			}
			else
			{
				while (temp)
				{
					next = fat32fat_link(temp,FAT32_VOID);
					if (next==FAT32_LAST_CLUSTER) break;
					temp = next;
				}
			}
		}
	}
	/* start writing */
	while (size>0)
	{
		if (!fat32_load_that(temp))
			return (int)FAT32_SECTOR_ERROR;
		while (loop<SECTOR_SIZE&&size>0)
		{
			part.sect.data[loop] = *data;
			loop++; data++; size--; fill++;
		}
		part.sect.fill = loop;
		if (sector_write()!=loop)
			return (int)FAT32_SECTOR_ERROR;
		if (!size) break;
		/* next sector? */
		temp = fat32_next();
		if (temp==FAT32_FAILURE) break;
		if (temp==FAT32_LAST_CLUSTER)
		{
			temp = fat32_get_empty_cluster();
			if (temp==FAT32_FAILURE) break;
			fat32fat_link(part.curr,temp);
			fat32fat_link(temp,FAT32_LAST_CLUSTER);
		}
		/* reset sector counter */
		loop = 0;
	}
	/* write new file size */
	pdir = fat32_find_name(name);
	if (!pdir) return (int)FAT32_FLAG_ERROR;
	pdir->size = fill;
	/* update that */
	if (sector_write()!=part.sect.fill)
		return (int)FAT32_SECTOR_ERROR;
	return 0;
}
/*----------------------------------------------------------------------------*/
