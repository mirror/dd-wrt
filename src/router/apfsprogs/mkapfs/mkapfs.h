/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _MKAPFS_H
#define _MKAPFS_H

#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <apfs/raw.h>

/* Filesystem parameters */
struct parameters {
	unsigned long	blocksize;	/* Block size */
	u64		block_count;	/* Number of blocks in the container */
	char		*label;		/* Volume label */
	char		*main_uuid;	/* Container UUID in standard format */
	char		*vol_uuid;	/* Volume UUID in standard format */
	bool		case_sensitive;	/* Is the filesystem case-sensitive? */
	bool		norm_sensitive;	/* Is it normalization-sensitive? */
};

/* Declarations for global variables */
extern struct parameters *param;	/* Filesystem parameters */
extern int fd;				/* File descriptor for the device */

/* Hardcoded transaction ids */
#define MKFS_XID	1

/* Hardcoded object ids */
#define	SPACEMAN_OID		APFS_OID_RESERVED_COUNT
#define REAPER_OID		(SPACEMAN_OID + 1)
#define FIRST_VOL_OID		(REAPER_OID + 1)
#define FIRST_VOL_CAT_ROOT_OID	(FIRST_VOL_OID + 1)
#define	IP_FREE_QUEUE_OID	(FIRST_VOL_CAT_ROOT_OID + 1)
#define MAIN_FREE_QUEUE_OID	(IP_FREE_QUEUE_OID + 1)

/**
 * cpoint_desc_blocks - Calculate the number of checkpoint descriptor blocks
 */
static inline u32 cpoint_desc_blocks(void)
{
	/*
	 * These numbers are a rough approximation of what I learned from
	 * testing newfs_apfs. I don't think the exact block counts matter.
	 */
	if (param->block_count < 512 * 1024 / 4) /* Up to 512M */
		return 8;
	if (param->block_count < 1024 * 1024 / 4) /* Up to 1G */
		return 12;
	if (param->block_count < 50 * 1024 * 1024 / 4) { /* Up to 50G */
		u32 off_512M = (param->block_count - 1024 * 1024 / 4) / (512 * 1024 / 4);
		return 20 + 60 * off_512M / 23;
	}

	/*
	 * From here on, newfs_apfs increases the number very slowly. I don't
	 * think it matters...
	 */
	return 280;
}

/**
 * cpoint_data_blocks - Calculate the number of checkpoint data blocks
 */
static inline u32 cpoint_data_blocks(void)
{
	/* I got these numbers from testing newfs_apfs */
	if (param->block_count < 4545)
		return 52;
	if (param->block_count < 13633)
		return 124;
	if (param->block_count < 36353)
		return 160 + 36 * ((param->block_count - 13633) / 4544);
	if (param->block_count < 131777)
		return 308 + 4 * ((param->block_count - 36353) / 4544);
	if (param->block_count < 262144)
		return 648 + 4 * ((param->block_count - 131777) / 4544);
	if (param->block_count == 262144) /* 1G is a special case, odd */
		return 992;

	/*
	 * At this point I got bored and the sizes stop matching exactly. The
	 * official fsck doesn't care. TODO?
	 */
	if (param->block_count < 1048576) { /* Up to 4G */
		u32 off_512M = (param->block_count - 262144) / 131072;
		return 1248 + 488 * off_512M + 4 * (((long long)param->block_count - (261280 + off_512M * 131776)) / 2272);
	}
	if (param->block_count < 4063232) { /* Up to 10G */
		u32 off_512M = (param->block_count - 1048576) / 131072;
		return 4112 + 256 * off_512M;
	}
	if (param->block_count < 13107200) { /* Up to 50G */
		u32 off_512M = (param->block_count - 4063232) / 131072;
		return 10000 + 256 * off_512M;
	}

	/*
	 * From here on, newfs_apfs increases the number very slowly. I don't
	 * think it matters...
	 */
	return 27672;
}

/* Description of the checkpoint areas */
#define CPOINT_DESC_BASE	(APFS_NX_BLOCK_NUM + 1)
#define CPOINT_DESC_BLOCKS	cpoint_desc_blocks()
#define CPOINT_DATA_BASE	(CPOINT_DESC_BASE + CPOINT_DESC_BLOCKS)
#define CPOINT_DATA_BLOCKS	cpoint_data_blocks()
#define CPOINT_END		(CPOINT_DATA_BASE + CPOINT_DATA_BLOCKS)

/* Block numbers calculated from the checkpoint areas */
#define CPOINT_MAP_BNO			CPOINT_DESC_BASE
#define CPOINT_SB_BNO			(CPOINT_DESC_BASE + 1)
#define REAPER_BNO			CPOINT_DATA_BASE
#define IP_FREE_QUEUE_BNO		(CPOINT_DATA_BASE + 1)
#define MAIN_FREE_QUEUE_BNO		(CPOINT_DATA_BASE + 2)
/* Spaceman comes last in the data area because it could need 2 blocks */
#define SPACEMAN_BNO			(CPOINT_DATA_BASE + 3)
#define MAIN_OMAP_BNO			CPOINT_END
#define MAIN_OMAP_ROOT_BNO		(CPOINT_END + 1)
#define FIRST_VOL_BNO			(CPOINT_END + 2)
#define FIRST_VOL_OMAP_BNO		(CPOINT_END + 3)
#define FIRST_VOL_OMAP_ROOT_BNO		(CPOINT_END + 4)
#define FIRST_VOL_CAT_ROOT_BNO		(CPOINT_END + 5)
#define FIRST_VOL_EXTREF_ROOT_BNO	(CPOINT_END + 6)
#define FIRST_VOL_SNAP_ROOT_BNO		(CPOINT_END + 7)
/* First ip bitmap comes last because the size is not hardcoded */
#define IP_BMAP_BASE			(CPOINT_END + 8)

extern __attribute__((noreturn)) void system_error(void);
extern __attribute__((noreturn)) void fatal(const char *message);

/**
 * get_zeroed_blocks - Map and zero contiguous filesystem blocks
 * @bno:	first block number
 * @count:	number of blocks
 *
 * Returns a pointer to the mapped area; the caller must unmap it after use.
 */
static inline void *get_zeroed_blocks(u64 bno, u64 count)
{
	void *blocks;
	size_t size = param->blocksize * count;

	if (size / count != param->blocksize)
		fatal("overflow detected on disk area mapping");

	blocks = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, bno * param->blocksize);
	if (blocks == MAP_FAILED)
		system_error();
	memset(blocks, 0, size);
	return blocks;
}

/**
 * get_zeroed_block - Map and zero a filesystem block
 * @bno: block number
 *
 * Returns a pointer to the mapped block; the caller must unmap it after use.
 */
static inline void *get_zeroed_block(u64 bno)
{
	return get_zeroed_blocks(bno, 1);
}

/**
 * get_timestamp - Get the current time in nanoseconds
 *
 * Calls clock_gettime(), so may not work with old versions of glibc.
 */
static inline u64 get_timestamp(void)
{
	struct timespec time;

	if (clock_gettime(CLOCK_REALTIME, &time))
		system_error();

	return (u64)time.tv_sec * NSEC_PER_SEC + time.tv_nsec;
}

#endif	/* _MKAPFS_H */
