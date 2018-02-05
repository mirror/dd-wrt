/*
 * Copyright (c) 2007 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
