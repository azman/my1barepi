/*----------------------------------------------------------------------------*/
#include "fat32.h"
#include "mbr.h"
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
	part.flag |= FAT32_SECTORR_ERROR;
	return FAT32_FAILURE;
}
/*----------------------------------------------------------------------------*/
word32_t fat32_root(void)
{
	/* reload root cluster */
	if (!fat32_load_that(part.root))
		return FAT32_FAILURE;
	/* first cluster in chain */
	part.cls1 = part.curr;
	return part.curr;
}
/*----------------------------------------------------------------------------*/
/* load fat32 information */
word32_t fat32_init(void)
{
	mbr_t* pmbr;
	fat32_vbr_t* pvbr;
	int loop, pick;
	/* setup flag, default opts */
	part.flag = FAT32_FLAG_OK;
	part._opt = 0; /** do i need this? */
	/* read first sector - look for partition table */
	if ((loop=sector_read(0))<=0)
	{
		part.flag |= FAT32_MBR_FAILED;
		return FAT32_FAILURE;
	}
	pmbr = (mbr_t*)part.sect.data;
	if (pmbr->ends[0]!=0x55||pmbr->ends[1]!=0xaa)
	{
		part.flag |= FAT32_MBR_INVALID;
		return FAT32_FAILURE;
	}
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
	{
		part.flag |= FAT32_NOT_FOUND;
		return FAT32_FAILURE;
	}
	if ((loop=sector_read(pmbr->ptab[pick].rsect))<=0)
	{
		part.flag |= FAT32_VBR_FAILED;
		return FAT32_FAILURE;
	}
	pvbr = (fat32_vbr_t*)part.sect.data;
	/* should have boot marker 0xaa55 */
	if (pvbr->ends[0]!=0x55||pvbr->ends[1]!=0xaa)
	{
		part.flag |= FAT32_VBR_INVALID;
		return FAT32_FAILURE;
	}
	/* 16-bit root entry should be zero for fat32 */
	if (pvbr->meta.biospb.rooten)
	{
		part.flag |= FAT32_INVALID_ENTRY;
		return FAT32_FAILURE;
	}
	/* FAT count is ALWAYS 2! */
	if (pvbr->meta.biospb.fatcnt!=2)
	{
		part.flag |= FAT32_INVALID_FAT32;
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
	/* save first partition sector in part.next (only used in fat32_next) */
	part.next = part.sect.offs;
	/* start to read root cluster */
	return fat32_root();
}
/*----------------------------------------------------------------------------*/
#define valid_1(that) ((that>='A'&&that<='Z')||(that>='0'&&that<='9'))
#define valid_2(that) (that>='a'&&that<='z')
#define valid_doschar(that) valid_1(that)?that:(valid_2(that)?that-0x20:'_')
/*----------------------------------------------------------------------------*/
/* create proper c-string dos filename from pdir 8+3 name entry */
void fat32dir_name(char* name)
{
	/* get 8+3 dos name from fat32_dir_t */
	int loop, test;
	for (loop=0;loop<8;loop++)
	{
		if (pdir->name[loop]==0x20)
		{
			name[loop] = 0x0;
			break;
		}
		else name[loop] = pdir->name[loop];
	}
	test = loop;
	if (pdir->name[8]&&pdir->name[8]!=0x20)
	{
		name[test++] = '.';
		name[test++] = pdir->name[8];
		for (loop=9;loop<11;loop++,test++)
		{
			if (!pdir->name[loop]||pdir->name[loop]==0x20)
			{
				name[test] = 0x0;
				break;
			}
			else name[test] = pdir->name[loop];
		}
	}
	name[test] = 0x0;
}
/*----------------------------------------------------------------------------*/
/* compare dos names, returns 1 if the same */
int fat32dir_name_cmp(char* name, char* what)
{
	int loop;
	for (loop=0;loop<FAT32_NAME_BUFFSIZE;loop++)
	{
		if (name[loop]!=what[loop]) break;
		if (!name[loop]) return 1;
	}
	return 0;
}
/*----------------------------------------------------------------------------*/
/* get/set fat cluster table */
word32_t fat32fat_link(word32_t curr, word32_t mark)
{
	word32_t temp, *pnxt;
	do
	{
		temp = curr/FAT32_FAT_ITEMS;
		if (sector_read(part.ifat+temp)!=SECTOR_SIZE)
		{
			part.flag |= FAT32_SECTORR_ERROR;
			break;
		}
		temp = curr%FAT32_FAT_ITEMS;
		pnxt = (word32_t*) part.sect.data;
		temp = pnxt[temp]; /* should not be zero? because linked! */
		if (mark!=FAT32_VOID)
		{
			pnxt[curr%FAT32_FAT_ITEMS] = mark;
			if (sector_write()!=part.sect.fill)
			{
				part.flag |= FAT32_SECTORW_ERROR;
				break;
			}
		}
		return temp;
	}
	while (0);
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
		part.flag |= FAT32_SECTORR_ERROR;
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
			fat32dir_name(dosname);
			if (fat32dir_name_cmp(dosname,name))
				return pdir;
		}
		temp = fat32_next();
	}
	return 0x0;
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
			part.flag |= FAT32_SECTORR_ERROR;
			return FAT32_FAILURE;
		}
		ptmp = (word32_t*) part.sect.data;
		for (isec=(temp==2)?temp:0;isec<FAT32_FAT_ITEMS;isec++,temp++)
			if (ptmp[isec]==FAT32_FREE_CLUSTER) return temp;
		isec = part.sect.offs+1; /* next sector of fat */
	}
	return FAT32_FAILURE;
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
/* open a file in fat32 fs - prepare for read XOR write */
word32_t fat32_open(my1file_t* file, char* name, int opts)
{
	int loop = 0;
	while (*name)
	{
		file->name[loop++] = (*name)=='.'?(*name):valid_doschar(*name);
		if (loop==12) break;
		name++;
	}
	file->name[loop] = 0x0;
	file->opts = opts & ~FILE_OPTS_OPENED;
	file->stat = 0;
	if (!fat32_find_name(file->name))
	{
		if (!(opts&FILE_OPTS_WRMODE)||!(opts&FILE_OPTS_CREATE))
		{
			file->stat = FILE_NOT_FOUND;
			return FAT32_FAILURE;
		}
		/* find first empy entry - should always find one? */
		pdir = fat32dir_find_free();
		if (!pdir)
		{
			part.flag |= FAT32_FULL_DIRENTRY;
			file->stat = FILE_CANNOT_CREATE;
			return FAT32_FAILURE;
		}
		/* fill in info */
		fat32dir_name_make(file->name);
		pdir->attr = 0x00; /* basic file */
		pdir->clhi = 0;
		pdir->clus = 0;
		pdir->size = 0;
		/* update that */
		if (sector_write()!=part.sect.fill)
		{
			part.flag |= FAT32_SECTORW_ERROR;
			file->stat = FILE_SECTOR_ERROR;
			return FAT32_FAILURE;
		}
	}
	else if (pdir->attr&FAT32_ATTR_SUB_DIR)
	{
		file->stat = FILE_NOT_FOUND;
		return FAT32_FAILURE;
	}
	file->init = fat32dir_cluster(pdir);
	file->curr = file->init;
	file->fpos = 0;
	file->size = pdir->size;
	file->fill = pdir->size%SECTOR_SIZE;
	if (file->opts&FILE_OPTS_WRMODE)
	{
		/* open for write@append */
		if (file->opts&FILE_OPTS_APPEND_MODE)
		{
			/* go to last cluster! */
			while (file->curr)
			{
				file->last = fat32fat_link(file->curr,FAT32_VOID);
				if (file->last==FAT32_LAST_CLUSTER) break;
				file->curr = file->last;
			}
			/* move file position to the end */
			file->fpos = file->size;
		}
		else
		{
			/* remove all content! */
			pdir->clhi = 0;
			pdir->clus = 0;
			pdir->size = 0;
			if (sector_write()!=part.sect.fill)
			{
				part.flag |= FAT32_SECTORW_ERROR;
				file->stat = FILE_CANNOT_CREATE;
				return FAT32_FAILURE;
			}
			/* release all clusters */
			while (file->init)
			{
				file->curr = fat32fat_link(file->init,FAT32_FREE_CLUSTER);
				if (file->curr==FAT32_LAST_CLUSTER) break;
				file->init = file->curr;
			}
			file->init = 0;
			file->curr = 0;
			file->size = 0;
		}
	}
	/* nothing else to do for read */
	file->opts |= FILE_OPTS_OPENED;
	return FAT32_VOID; /* non-zero value is OK */
}
/*----------------------------------------------------------------------------*/
/* simply mark a file as closed - if a stream, should commit here */
word32_t fat32_close(my1file_t* file)
{
	if (file->opts&FILE_OPTS_OPENED)
		file->opts &= ~FILE_OPTS_OPENED;
	return FAT32_VOID; /* non-zero => ok! */
}
/*----------------------------------------------------------------------------*/
/* change into a dir within current working dir */
word32_t fat32_chdir(my1file_t* file, char* name)
{
	int loop = 0;
	while (*name)
	{
		file->name[loop++] = (*name)=='.'?(*name):valid_doschar(*name);
		if (loop==12) break;
		name++;
	}
	file->name[loop] = 0x0;
	file->opts &= ~FILE_OPTS_OPENED; /* just in case */
	file->stat = 0;
	if (!fat32_find_name(file->name))
	{
		file->stat = FILE_DIR_NOT_FOUND;
		return FAT32_FAILURE;
	}
	if (!(pdir->attr&FAT32_ATTR_SUB_DIR))
	{
		file->stat = FILE_DIR_NOT_FOUND;
		return FAT32_FAILURE;
	}
	if (!fat32_load_that(fat32dir_cluster(pdir)))
	{
		file->stat = FILE_SECTOR_ERROR;
		return FAT32_FAILURE;
	}
	/* first cluster in chain */
	part.cls1 = part.curr;
	return part.curr;
}
/*----------------------------------------------------------------------------*/
#define FILE_READY2_WRITE (FILE_OPTS_OPENED|FILE_OPTS_WRMODE)
/*----------------------------------------------------------------------------*/
/* write to opened file */
int fat32_write(my1file_t* file, byte08_t* data, int size)
{
	if (!(file->opts&FILE_READY2_WRITE)) return 0;
	file->last = 0;
	file->stat = 0;
	/* start writing */
	while (size>0)
	{
		if (!file->curr) /* most probably size IS 0! */
		{
			/* find first empy cluster */
			file->curr = fat32_get_empty_cluster();
			if (file->curr==FAT32_FAILURE)
			{
				part.flag |= FAT32_FULL_CLUSTER;
				file->stat = FILE_SECTOR_ERROR;
				return FAT32_FAILURE;
			}
			/* mark that as used */
			fat32fat_link(file->curr,FAT32_LAST_CLUSTER);
			/* update pdir entry */
			pdir = fat32_find_name(file->name);
			if (!pdir)
			{
				file->stat = FILE_ENTRY_LOST;
				return FAT32_FAILURE;
			}
			/* update cluster info */
			pdir->clhi = (file->curr>>16)&0xffff;
			pdir->clus = file->curr&0xffff;
			/* update that */
			if (sector_write()!=part.sect.fill)
			{
				file->stat = FILE_SECTOR_ERROR;
				return FAT32_FAILURE;
			}
			file->init = file->curr;
		}
		if (!fat32_load_that(file->curr))
		{
			file->stat = FILE_SECTOR_ERROR;
			return FAT32_FAILURE;
		}
		while (file->fill<SECTOR_SIZE&&size>0)
		{
			part.sect.data[file->fill] = *data;
			file->fill++; data++; size--;
			file->last++; file->fpos++;
		}
		if (file->fpos>file->size)
			file->size = file->fpos;
		part.sect.fill = file->fill;
		if (sector_write()!=file->fill)
		{
			file->stat = FILE_SECTOR_ERROR;
			break;
		}
		if (!size) break;
		/* load next sector/cluster */
		file->curr = fat32_next();
		if (file->curr==FAT32_FAILURE)
		{
			file->stat = FILE_SECTOR_ERROR;
			break;
		}
		if (file->curr==FAT32_LAST_CLUSTER)
		{
			file->curr = fat32_get_empty_cluster();
			if (file->curr==FAT32_FAILURE)
			{
				file->stat = FILE_SECTOR_ERROR;
				break;
			}
			fat32fat_link(part.curr,file->curr);
			fat32fat_link(file->curr,FAT32_LAST_CLUSTER);
		}
		/* reset sector counter */
		file->fill = 0;
	}
	if (!file->stat)
	{
		/* write new file size */
		pdir = fat32_find_name(file->name);
		if (!pdir)
		{
			file->stat = FILE_ENTRY_LOST;
			return FAT32_FAILURE;
		}
		pdir->size = file->size;
		/* update that */
		if (sector_write()!=part.sect.fill)
		{
			file->stat = FILE_SECTOR_ERROR;
			return FAT32_FAILURE;
		}
	}
	return file->last;
}
/*----------------------------------------------------------------------------*/
/* read from opened file */
int fat32_read(my1file_t* file, byte08_t* data, int size)
{
	if (!(file->opts&FILE_OPTS_OPENED)||(file->opts&FILE_OPTS_WRMODE))
		return 0;
	file->last = 0;
	file->stat = 0;
	if (!file->size) return 0; /* just in case :p */
	/* start reading */
	if (!fat32_load_that(file->curr))
	{
		file->stat = FILE_SECTOR_ERROR;
		return FAT32_FAILURE;
	}
	file->tpos = file->fpos;
	file->fpos = 0;
	while (file->curr!=FAT32_FAILURE&&file->curr!=FAT32_LAST_CLUSTER)
	{
		file->fill = 0;
		while (file->fill<SECTOR_SIZE&&file->fpos<file->size)
		{
			file->fpos++;
			if (file->fpos>file->tpos)
			{
				data[file->last++] = part.sect.data[file->fill];
				if (file->last==size)
					return file->last;
			}
			file->fill++;
		}
		if (file->fpos==file->size)
			break;
		file->curr = fat32_next();
	}
	return file->last;
}
/*----------------------------------------------------------------------------*/
