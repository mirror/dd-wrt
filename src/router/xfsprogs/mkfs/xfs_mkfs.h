/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
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
#ifndef __XFS_MKFS_H__
#define	__XFS_MKFS_H__

#define XFS_DFL_SB_VERSION_BITS \
                (XFS_SB_VERSION_NLINKBIT | \
                 XFS_SB_VERSION_EXTFLGBIT | \
                 XFS_SB_VERSION_DIRV2BIT)

#define XFS_SB_VERSION_MKFS(ia,dia,log2,attr1,sflag,ci,more) (\
	((ia)||(dia)||(log2)||(attr1)||(sflag)||(ci)||(more)) ? \
	( XFS_SB_VERSION_4 |						\
		((ia) ? XFS_SB_VERSION_ALIGNBIT : 0) |			\
		((dia) ? XFS_SB_VERSION_DALIGNBIT : 0) |		\
		((log2) ? XFS_SB_VERSION_LOGV2BIT : 0) |		\
		((attr1) ? XFS_SB_VERSION_ATTRBIT : 0) |		\
		((sflag) ? XFS_SB_VERSION_SECTORBIT : 0) |		\
		((ci) ? XFS_SB_VERSION_BORGBIT : 0) |			\
		((more) ? XFS_SB_VERSION_MOREBITSBIT : 0) |		\
	        XFS_DFL_SB_VERSION_BITS |                               \
	0 ) : XFS_SB_VERSION_1 )

#define XFS_SB_VERSION2_MKFS(lazycount, attr2, projid32bit, parent) (\
	((lazycount) ? XFS_SB_VERSION2_LAZYSBCOUNTBIT : 0) |		\
	((attr2) ? XFS_SB_VERSION2_ATTR2BIT : 0) |			\
	((projid32bit) ? XFS_SB_VERSION2_PROJID32BIT : 0) |		\
	((parent) ? XFS_SB_VERSION2_PARENTBIT : 0) |			\
	0 )

#define	XFS_DFL_BLOCKSIZE_LOG	12		/* 4096 byte blocks */
#define	XFS_DINODE_DFL_LOG	8		/* 256 byte inodes */
#define	XFS_MIN_DATA_BLOCKS	100
#define	XFS_MIN_INODE_PERBLOCK	2		/* min inodes per block */
#define	XFS_DFL_IMAXIMUM_PCT	25		/* max % of space for inodes */
#define	XFS_IFLAG_ALIGN		1		/* -i align defaults on */
#define	XFS_MIN_REC_DIRSIZE	12		/* 4096 byte dirblocks (V2) */
#define	XFS_DFL_DIR_VERSION	2		/* default directory version */
#define	XFS_DFL_LOG_SIZE	1000		/* default log size, blocks */
#define	XFS_MIN_LOG_FACTOR	3		/* min log size factor */
#define	XFS_DFL_LOG_FACTOR	16		/* default log size, factor */
						/* with max trans reservation */
#define XFS_MAX_INODE_SIG_BITS	32		/* most significant bits in an
						 * inode number that we'll
						 * accept w/o warnings
						 */

#define XFS_AG_BYTES(bblog)	((long long)BBSIZE << (bblog))
#define	XFS_AG_MIN_BYTES	((XFS_AG_BYTES(15)))	/* 16 MB */
#define XFS_AG_MIN_BLOCKS(blog)	((XFS_AG_BYTES(15)) >> (blog))
#define XFS_AG_MAX_BLOCKS(blog)	((XFS_AG_BYTES(31) - 1) >> (blog))

#define XFS_MAX_AGNUMBER	((xfs_agnumber_t)(NULLAGNUMBER - 1))


/* xfs_mkfs.c */
extern int isdigits (char *str);
extern long long cvtnum (unsigned int blocksize,
			 unsigned int sectorsize, char *s);

/* proto.c */
extern char *setup_proto (char *fname);
extern void parse_proto (xfs_mount_t *mp, struct fsxattr *fsx, char **pp);
extern void res_failed (int err);

/* maxtrres.c */
extern int max_trans_res (int dirversion,
		int sectorlog, int blocklog, int inodelog, int dirblocklog);

#endif	/* __XFS_MKFS_H__ */
