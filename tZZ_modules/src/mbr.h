/*----------------------------------------------------------------------------*/
#ifndef __MY1MBRH__
#define __MY1MBRH__
/*----------------------------------------------------------------------------*/
#include "types.h"
/*----------------------------------------------------------------------------*/
typedef struct _ptable_t
{
	byte08_t bflag; /* active partition flag (0x80: active, 0x00: inactive) */
	byte08_t ihead; /* init head */
	byte08_t isect; /* sector: sect&0x3f => 0-63 */
	byte08_t ilcyl; /* cylinder: ((lcyl&0xff)|((sect&0xc0) << 2)) => 0-1023 */
	byte08_t sysid; /* (0x0c = W95 FAT32 LBA, 0x83 = Linux) */
	byte08_t ehead; /* ends head */
	byte08_t esect;
	byte08_t elcyl;
	word32_t rsect; /** lba first sector */
	word32_t tsect; /** total sector */
}
ptable_t; /* partition table entry - 16 bytes! */
/*----------------------------------------------------------------------------*/
typedef struct _mbr_t
{
	byte08_t exe1[218]; /* executables */
	byte08_t tag1[2]; /* 218 - always 0x00 0x00? */
	byte08_t stmp[4]; /* 220 - disk timestamp (format?) */
	byte08_t exe2[216]; /* 224 - executables */
	byte08_t dsig[4];   /* 440 - 4 bytes disk signature */
	byte08_t tag2[2];   /* 444 - always 0x00 0x00? */
	struct _ptable_t ptab[4]; /* 446 - 4 primary partitions */
	byte08_t ends[2];   /* 510 - 0x55,0xaa => 0xaa55 (little endian) */
} __attribute__ ((packed))
mbr_t;
/*----------------------------------------------------------------------------*/
#endif /* __MY1MBRH__ */
/*----------------------------------------------------------------------------*/
