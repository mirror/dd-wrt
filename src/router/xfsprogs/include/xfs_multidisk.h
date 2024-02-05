// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef __XFS_MULTIDISK_H__
#define	__XFS_MULTIDISK_H__

#define XFS_DFL_SB_VERSION_BITS \
                (XFS_SB_VERSION_NLINKBIT | \
                 XFS_SB_VERSION_EXTFLGBIT | \
                 XFS_SB_VERSION_DIRV2BIT)

#define	XFS_DFL_BLOCKSIZE_LOG	12		/* 4096 byte blocks */
#define	XFS_DINODE_DFL_LOG	8		/* 256 byte inodes */
#define	XFS_DINODE_DFL_CRC_LOG	9		/* 512 byte inodes for CRCs */
#define	XFS_MIN_INODE_PERBLOCK	2		/* min inodes per block */
#define	XFS_DFL_IMAXIMUM_PCT	25		/* max % of space for inodes */
#define	XFS_MIN_REC_DIRSIZE	12		/* 4096 byte dirblocks (V2) */
#define XFS_MAX_INODE_SIG_BITS	32		/* most significant bits in an
						 * inode number that we'll
						 * accept w/o warnings
						 */

#define XFS_AG_BYTES(bblog)		((long long)BBSIZE << (bblog))
#define	XFS_MIN_DATA_BLOCKS(cfg)	(XFS_AG_MIN_BLOCKS((cfg)->blocklog))
#define	XFS_AG_MIN_BYTES		((XFS_AG_BYTES(15)))	/* 16 MB */
#define	XFS_AG_MAX_BYTES		((XFS_AG_BYTES(31)))	/* 1 TB */
#define XFS_AG_MIN_BLOCKS(blog)		(XFS_AG_MIN_BYTES >> (blog))
#define XFS_AG_MAX_BLOCKS(blog)		((XFS_AG_MAX_BYTES - 1) >> (blog))

#define XFS_MAX_AGNUMBER		((xfs_agnumber_t)(NULLAGNUMBER - 1))

/*
 * These values define what we consider a "multi-disk" filesystem. That is, a
 * filesystem that is likely to be made up of multiple devices, and hence have
 * some level of parallelism available to it at the IO level.
 */
#define XFS_MULTIDISK_AGLOG		5	/* 32 AGs */
#define XFS_NOMULTIDISK_AGLOG		2	/* 4 AGs */
#define XFS_MULTIDISK_AGCOUNT		(1 << XFS_MULTIDISK_AGLOG)

#endif	/* __XFS_MULTIDISK_H__ */
