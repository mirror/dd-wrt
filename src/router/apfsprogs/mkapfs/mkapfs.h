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

/* String to identify the program and its version */
#define MKFS_ID_STRING	"mkapfs for linux, version 0.1"

/* Hardcoded transaction ids */
#define MKFS_XID	1

/* Hardcoded object ids */
#define	SPACEMAN_OID		APFS_OID_RESERVED_COUNT
#define REAPER_OID		(SPACEMAN_OID + 1)
#define FIRST_VOL_OID		(REAPER_OID + 1)
#define FIRST_VOL_CAT_ROOT_OID	(FIRST_VOL_OID + 1)
#define	IP_FREE_QUEUE_OID	(FIRST_VOL_CAT_ROOT_OID + 1)
#define MAIN_FREE_QUEUE_OID	(IP_FREE_QUEUE_OID + 1)

/*
 * Constants describing the checkpoint areas; these are hardcoded for now, but
 * should actually change with the container size.
 */
#define CPOINT_DESC_BASE	(APFS_NX_BLOCK_NUM + 1)
#define CPOINT_DESC_BLOCKS	64
#define CPOINT_DATA_BASE	(CPOINT_DESC_BASE + CPOINT_DESC_BLOCKS)
#define CPOINT_DATA_BLOCKS	5904

/*
 * Constants describing the internal pool; these are hardcoded for now, but
 * should actually change with the container size.
 */
#define IP_BMAP_BASE	21000				/* First ip bitmap */
#define IP_BMAP_BLOCKS	16				/* Ip bitmap count */
#define IP_BASE		(IP_BMAP_BASE + IP_BMAP_BLOCKS)	/* Start of pool */

/* Hardcoded block numbers */
#define CPOINT_MAP_BNO			CPOINT_DESC_BASE
#define CPOINT_SB_BNO			(CPOINT_DESC_BASE + 1)
#define REAPER_BNO			CPOINT_DATA_BASE
#define SPACEMAN_BNO			(CPOINT_DATA_BASE + 1)
#define	IP_FREE_QUEUE_BNO		(CPOINT_DATA_BASE + 2)
#define MAIN_FREE_QUEUE_BNO		(CPOINT_DATA_BASE + 3)
#define MAIN_OMAP_BNO			20000
#define MAIN_OMAP_ROOT_BNO		20001
#define FIRST_VOL_BNO			20002
#define FIRST_VOL_OMAP_BNO		20003
#define FIRST_VOL_OMAP_ROOT_BNO		20004
#define FIRST_VOL_CAT_ROOT_BNO		20005
#define FIRST_VOL_EXTREF_ROOT_BNO	20006
#define FIRST_VOL_SNAP_ROOT_BNO		20007

/* Declarations for global variables */
extern struct parameters *param;	/* Filesystem parameters */
extern int fd;				/* File descriptor for the device */

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
