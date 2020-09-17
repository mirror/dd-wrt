// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2007 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef _XFS_METADUMP_H_
#define _XFS_METADUMP_H_

#define	XFS_MD_MAGIC		0x5846534d	/* 'XFSM' */

typedef struct xfs_metablock {
	__be32		mb_magic;
	__be16		mb_count;
	uint8_t		mb_blocklog;
	uint8_t		mb_info;
	/* followed by an array of xfs_daddr_t */
} xfs_metablock_t;

/* These flags are informational only, not backwards compatible */
#define XFS_METADUMP_INFO_FLAGS	(1 << 0) /* This image has informative flags */
#define XFS_METADUMP_OBFUSCATED	(1 << 1)
#define XFS_METADUMP_FULLBLOCKS	(1 << 2)
#define XFS_METADUMP_DIRTYLOG	(1 << 3)

#endif /* _XFS_METADUMP_H_ */
