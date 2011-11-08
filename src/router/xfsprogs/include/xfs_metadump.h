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
	__uint8_t	mb_blocklog;
	__uint8_t	mb_reserved;
	/* followed by an array of xfs_daddr_t */
} xfs_metablock_t;

#endif /* _XFS_METADUMP_H_ */
