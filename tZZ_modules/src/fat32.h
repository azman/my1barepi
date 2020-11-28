/*----------------------------------------------------------------------------*/
#ifndef __MY1FAT32_H__
#define __MY1FAT32_H__
/*----------------------------------------------------------------------------*/
#include "types.h"
/*----------------------------------------------------------------------------*/
#define PARTID_FAT32 0x0b
#define PARTID_FAT32_LBA 0x0c
/*----------------------------------------------------------------------------*/
/* fat32 cluster index is actually 28-bits? */
#define FAT32_NEXT_CLUSTER_MASK 0x0FFFFFFF
#define FAT32_LAST_CLUSTER FAT32_NEXT_CLUSTER_MASK
#define FAT32_FREE_CLUSTER 0
/*----------------------------------------------------------------------------*/
#define FAT32_INVALID 0xFFFFFFFF
#define FAT32_FAILURE 0x00000000
#define FAT32_VOID FAT32_INVALID
#define FAT32_FAIL FAT32_FAILURE
/*----------------------------------------------------------------------------*/
struct _fat_common_t
{
	byte08_t jmpx[3]; /* jmp short ?? ; nop [ eb ?? 90 ] OR
		jmp near [ 0xe9 0x?? 0x?? ] */
	char oem_id[8];   /* 03 - oem id string */
	/** actual start of bpb */
	word16_t bpsect;  /* 11 - bytes per sector : 512,1024,2048,4096 */
	byte08_t spclus;  /* 13 - sector per cluster : 1,2,4,8,16,32,64,128 */
	word16_t rsccnt;  /* 14 - # of reserved sectors (FAT12/16=>1, FAT32=>32) */
	byte08_t fatcnt;  /* 16 - # of fat (always 2?) */
	word16_t rooten;  /* 17 - root entries (fat16=>512,fat32=>0) */
	word16_t sctsma;  /* 19 - sectors (fat32=>0, also 0 if smaller than 32M?) */
	byte08_t medtyp;  /* 21 - media type (f0-floppy, f8-hard disk) */
	word16_t scpfat;  /* 22 - sectors per fat (on small vols, fat32=>0) */
	word16_t scptrk;  /* 24 - sectors per track (63?) */
	word16_t heads_;  /* 26 - heads */
	word16_t hidsec;  /* 28 - hidden sectors */
} __attribute__((__packed__)); /* 30 - 3 bytes = 27 bytes */
/** NOTE: cluster size should be <= 32K (rare cases of 64K ok) */
/*----------------------------------------------------------------------------*/
/* maybe support this as well? */
struct _fat_fat16_t
{
	struct _fat_common_t biospb; /* bios parameter block */
	word16_t hidsec; /* 30 - hidden sectors (extension?) */
	word32_t sctlrg; /* 32 - sectors (large volumes) */
	byte08_t drvnum; /* 36 - logical drive num - for use with int13 */
	byte08_t rsvd_1; /* 37 - reserved */
	byte08_t extsig; /* 38 - (0x29) ext sig indicate presence (next 3 fields) */
	word32_t sernum; /* 39 - serial number for partition */
	char vollbl[11]; /* 43 - volume label or "NO NAME    " */
	char fstype[8];  /* 54 - fs type FAT12/FAT16/FAT or all 0 */
} __attribute__((__packed__)); /* 62 - 3 bytes = 59 bytes */
/*----------------------------------------------------------------------------*/
struct _fat_fat32_t
{
	struct _fat_common_t biospb; /* bios parameter block */
	word16_t hidsec; /* 30 - hidden sectors (extension?) */
	word32_t sctlrg; /* 32 - sectors (large volumes) */
	word32_t scpfat; /* 36 - sectors per fat */
	word16_t m_flag; /* 40 - mirror flags (b15-b8:rsvd) (b6-b4:rsvd)
						(b7:1=>single active fat,0=>all fats updated @runtime)
						(b3-b0: # of active fat if b7=1) */
	word16_t fsvers; /* 42 - fs version */
	word32_t clroot; /* 44 - first cluster of root directory (usu. 2) */
	word16_t fsinfo; /* 48 - fs info sector num in fat rsvd area (usu. 1) */
	word16_t backbs; /* 50 - backup boot sector (0xFFFF if none, usu. 6) */
	byte08_t rsvd_1[12]; /* 52 - reserved */
	byte08_t drvnum; /* 64 - logical drive num - for use with int13 */
	byte08_t rsvd_2; /* 65 - reserved */
	byte08_t extsig; /* 66 - (0x29) ext sig indicate presence (next 3 fields) */
	word32_t sernum; /* 67 - serial number for partition */
	char vollbl[11]; /* 71 - volume label or "NO NAME    " */
	char fstype[8];  /* 82 - fs type "FAT32   " */
} __attribute__((__packed__)); /* 90 - 3 bytes = 87 bytes */
/*----------------------------------------------------------------------------*/
struct _fat32_sector0_t /* volume boot record (vbr!)  */
{
	struct _fat_fat32_t meta; /* 90 bytes */
	byte08_t unused[418]; /* meta might overflow into this => 418 bytes */
	byte08_t chk1; /* always 0x00? */
	byte08_t drvnum; /* physical drive num => dos3 era, now always 0x00? */
	byte08_t ends[2]; /* 510 - 0x55,0xaa => 0xaa55 (little endian) */
} __attribute__((__packed__));
/*----------------------------------------------------------------------------*/
typedef struct _fat32_sector0_t fat32_vbr_t;
/*----------------------------------------------------------------------------*/
struct _fat32_sector1_t /* fs info sector : IS THIS RELIABLE? */
{
	byte08_t tag1[4]; /* 0x52 0x52 0x61 0x41 - RRaA */
	byte08_t rsv1[480]; /* reserved */
	byte08_t tag2[4]; /* 0x72 0x72 0x41 0x61 - rrAa */
	word32_t free; /* number of free clusters (0xffffffff=unknown) */
	word32_t last; /* last clusters assigned (0xffffffff=unknown) */
	byte08_t ends[4]; /* 0xaa550000 (little endian) */
} __attribute__((__packed__));
/*----------------------------------------------------------------------------*/
typedef struct _fat32_sector1_t fat32_inf_t;
/*----------------------------------------------------------------------------*/
#define FAT32_ATTR_READ_ONLY 0x01
#define FAT32_ATTR_HIDDEN 0x02
#define FAT32_ATTR_SYSTEM 0x04
#define FAT32_ATTR_VOLUMELBL 0x08
#define FAT32_ATTR_SUB_DIR 0x10
#define FAT32_ATTR_ARCHIVE 0x20
/*----------------------------------------------------------------------------*/
#define FAT32_NAME_BUFFSIZE 16
/*----------------------------------------------------------------------------*/
struct _fat32_dir_t
{
	char name[11]; /* 8+3, whitespace (NOT NULL!) padded for 8 */
	byte08_t attr; /* 6-bits actually */
	byte08_t non1[8]; /* reserved */
	word16_t clhi; /* first cluster - hi 2 bytes */
	word16_t time; /* 5/6/5 => hh/mm/ss */
	word16_t date; /* 7/4/5 => yy-1980/mm/dd */
	word16_t clus; /* first cluster - low 2 bytes */
	word32_t size;
} __attribute__((__packed__)); /* 32-bytes directory entry - also file? */
/*----------------------------------------------------------------------------*/
typedef struct _fat32_dir_t fat32_dir_t;
/*----------------------------------------------------------------------------*/
#include "sector.h"
/*----------------------------------------------------------------------------*/
#define FAT32_FAT_ITEMS (SECTOR_SIZE/4)
/*----------------------------------------------------------------------------*/
#define FAT32_FLAG_OK 0
#define FAT32_FLAG_ERROR (~(~0U>>1))
#define FAT32_MBR_FAILED     (FAT32_FLAG_ERROR|0x00000001)
#define FAT32_MBR_INVALID    (FAT32_FLAG_ERROR|0x00000002)
#define FAT32_NOT_FOUND      (FAT32_FLAG_ERROR|0x00000004)
#define FAT32_VBR_FAILED     (FAT32_FLAG_ERROR|0x00000010)
#define FAT32_VBR_INVALID    (FAT32_FLAG_ERROR|0x00000020)
#define FAT32_SECTORR_ERROR  (FAT32_FLAG_ERROR|0x00000040)
#define FAT32_SECTORW_ERROR  (FAT32_FLAG_ERROR|0x00000080)
#define FAT32_INVALID_ENTRY  (FAT32_FLAG_ERROR|0x00000100)
#define FAT32_INVALID_FAT32  (FAT32_FLAG_ERROR|0x00000200)
#define FAT32_INVALID_SSIZE  (FAT32_FLAG_ERROR|0x00000400)
#define FAT32_FULL_DIRENTRY  (FAT32_FLAG_ERROR|0x00000800)
#define FAT32_FULL_CLUSTER   (FAT32_FLAG_ERROR|0x00001000)
/*----------------------------------------------------------------------------*/
#define FAT32_LIST_RAWENTRY 0x01
/*----------------------------------------------------------------------------*/
typedef struct _my1fat32_t
{
	word32_t flag;
	word32_t ifat; /* starting sector for fat */
	word32_t data; /* starting sector for data cluster */
	word32_t root; /* cluster index for root - 2? */
	word16_t _opt;
	byte08_t _spc; /* cluster size in sector (sector per cluster) */
	byte08_t _sec; /* currect sector index in current cluster */
	word32_t cls1; /* first cluster in this cluster chain */
	word32_t curr; /* current cluster for loaded sector */
	word32_t next; /* next cluster in chain output of fat32_next */
	my1sector_t sect; /* sector buffer */
}
my1fat32_t;
/*----------------------------------------------------------------------------*/
#define FILE_OPTS_OPENED 0x08
#define FILE_OPTS_DOREAD 0x00
#define FILE_OPTS_WRMODE 0x01
#define FILE_OPTS_CREATE_MODE 0x02
#define FILE_OPTS_APPEND_MODE 0x04
#define FILE_OPTS_CREATE (FILE_OPTS_WRMODE|FILE_OPTS_CREATE_MODE)
#define FILE_OPTS_APPEND (FILE_OPTS_WRMODE|FILE_OPTS_APPEND_MODE)
/*----------------------------------------------------------------------------*/
#define FILE_NOT_FOUND -1
#define FILE_CANNOT_CREATE -2
#define FILE_DIR_NOT_FOUND -3
#define FILE_SECTOR_ERROR  -4
#define FILE_ENTRY_LOST -5
/*----------------------------------------------------------------------------*/
typedef struct _my1file_t
{
	char name[FAT32_NAME_BUFFSIZE];
	word32_t init; /* initial physical point - fs-dependent? */
	word32_t curr; /* current physical point - fs-dependent? */
	word32_t opts; /* file options */
	word32_t size; /* current size */
	word32_t last; /* counter for last access */
	word32_t fpos; /* file byte position - for reading */
	word32_t tpos; /* temp for reading */
	word16_t fill; /* used bytes in current sector */
	word16_t stat; /* error status for file ops */
}
my1file_t;
/*----------------------------------------------------------------------------*/
word32_t fat32_init(void);
word32_t fat32_root(void); /* back to root */
word32_t fat32_open(my1file_t* file, char* name, int opts);
word32_t fat32_close(my1file_t* file);
word32_t fat32_chdir(my1file_t* file, char* name); /* single dir in cwd */
int fat32_write(my1file_t* file, byte08_t* data, int size);
int fat32_read(my1file_t* file, byte08_t* data, int size);
/*----------------------------------------------------------------------------*/
#endif /* __MY1FAT32_H__ */
/*----------------------------------------------------------------------------*/
