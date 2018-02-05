/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.
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

#ifndef __XFS_MOUNT_H__
#define __XFS_MOUNT_H__

struct xfs_inode;
struct xfs_buftarg;
struct xfs_dir_ops;
struct xfs_da_geometry;

/*
 * Define a user-level mount structure with all we need
 * in order to make use of the numerous XFS_* macros.
 */
typedef struct xfs_mount {
	xfs_sb_t		m_sb;		/* copy of fs superblock */
#define m_icount	m_sb.sb_icount
#define m_ifree		m_sb.sb_ifree
#define m_fdblocks	m_sb.sb_fdblocks
	char			*m_fsname;	/* filesystem name */
	int			m_bsize;	/* fs logical block size */
	xfs_agnumber_t		m_agfrotor;	/* last ag where space found */
	xfs_agnumber_t		m_agirotor;	/* last ag dir inode alloced */
	xfs_agnumber_t		m_maxagi;	/* highest inode alloc group */
	uint			m_rsumlevels;	/* rt summary levels */
	uint			m_rsumsize;	/* size of rt summary, bytes */
	struct xfs_inode	*m_rbmip;	/* pointer to bitmap inode */
	struct xfs_inode	*m_rsumip;	/* pointer to summary inode */
	struct xfs_buftarg	*m_ddev_targp;
	struct xfs_buftarg	*m_logdev_targp;
	struct xfs_buftarg	*m_rtdev_targp;
#define m_dev		m_ddev_targp
#define m_logdev	m_logdev_targp
#define m_rtdev		m_rtdev_targp
	uint8_t			m_dircook_elog;	/* log d-cookie entry bits */
	uint8_t			m_blkbit_log;	/* blocklog + NBBY */
	uint8_t			m_blkbb_log;	/* blocklog - BBSHIFT */
	uint8_t			m_sectbb_log;	/* sectorlog - BBSHIFT */
	uint8_t			m_agno_log;	/* log #ag's */
	uint8_t			m_agino_log;	/* #bits for agino in inum */
	uint			m_inode_cluster_size;/* min inode buf size */
	uint			m_blockmask;	/* sb_blocksize-1 */
	uint			m_blockwsize;	/* sb_blocksize in words */
	uint			m_blockwmask;	/* blockwsize-1 */
	uint			m_alloc_mxr[2];	/* XFS_ALLOC_BLOCK_MAXRECS */
	uint			m_alloc_mnr[2];	/* XFS_ALLOC_BLOCK_MINRECS */
	uint			m_bmap_dmxr[2];	/* XFS_BMAP_BLOCK_DMAXRECS */
	uint			m_bmap_dmnr[2];	/* XFS_BMAP_BLOCK_DMINRECS */
	uint			m_inobt_mxr[2];	/* XFS_INOBT_BLOCK_MAXRECS */
	uint			m_inobt_mnr[2];	/* XFS_INOBT_BLOCK_MINRECS */
	uint			m_rmap_mxr[2];	/* max rmap btree records */
	uint			m_rmap_mnr[2];	/* min rmap btree records */
	uint			m_refc_mxr[2];	/* max refc btree records */
	uint			m_refc_mnr[2];	/* min refc btree records */
	uint			m_ag_maxlevels;	/* XFS_AG_MAXLEVELS */
	uint			m_bm_maxlevels[2]; /* XFS_BM_MAXLEVELS */
	uint			m_in_maxlevels;	/* XFS_IN_MAXLEVELS */
	uint			m_rmap_maxlevels; /* max rmap btree levels */
	uint			m_refc_maxlevels; /* max refc btree levels */
	xfs_extlen_t		m_ag_prealloc_blocks; /* reserved ag blocks */
	uint			m_alloc_set_aside; /* space we can't use */
	uint			m_ag_max_usable; /* max space per AG */
	struct radix_tree_root	m_perag_tree;
	uint			m_flags;	/* global mount flags */
	bool			m_inotbt_nores; /* no per-AG finobt resv. */
	uint			m_qflags;	/* quota status flags */
	uint			m_attroffset;	/* inode attribute offset */
	int			m_ialloc_inos;	/* inodes in inode allocation */
	int			m_ialloc_blks;	/* blocks in inode allocation */
	int			m_ialloc_min_blks; /* min blocks in sparse inode
						    * allocation */
	int			m_litino;	/* size of inode union area */
	int			m_inoalign_mask;/* mask sb_inoalignmt if used */
	struct xfs_trans_resv	m_resv;		/* precomputed res values */
	uint64_t		m_maxicount;	/* maximum inode count */
	int			m_dalign;	/* stripe unit */
	int			m_swidth;	/* stripe width */
	int			m_sinoalign;	/* stripe unit inode alignmnt */
	const struct xfs_nameops *m_dirnameops;	/* vector of dir name ops */

	struct xfs_da_geometry	*m_dir_geo;	/* directory block geometry */
	struct xfs_da_geometry	*m_attr_geo;	/* attribute block geometry */
	const struct xfs_dir_ops *m_dir_inode_ops; /* vector of dir inode ops */
	const struct xfs_dir_ops *m_nondir_inode_ops; /* !dir inode ops */
#define M_DIROPS(mp)	((mp)->m_dir_inode_ops)

	/*
	 * anonymous struct to allow xfs_dquot_buf.c to compile.
	 * Pointer is always null in userspace, so code does not use it at all
	 */
	struct {
		int	qi_dqperchunk;
	}			*m_quotainfo;

	/*
	 * xlog is defined in libxlog and thus is not intialized by libxfs. This
	 * allows an application to initialize and store a reference to the log
	 * if warranted.
	 */
	struct xlog		*m_log;
} xfs_mount_t;

/* per-AG block reservation data structures*/
enum xfs_ag_resv_type {
	XFS_AG_RESV_NONE = 0,
	XFS_AG_RESV_METADATA,
	XFS_AG_RESV_AGFL,
};

struct xfs_ag_resv {
	/* number of blocks originally reserved here */
	xfs_extlen_t	ar_orig_reserved;
	/* number of blocks reserved here */
	xfs_extlen_t	ar_reserved;
	/* number of blocks originally asked for */
	xfs_extlen_t	ar_asked;
};

/*
 * Per-ag incore structure, copies of information in agf and agi,
 * to improve the performance of allocation group selection.
 */
typedef struct xfs_perag {
	struct xfs_mount *pag_mount;	/* owner filesystem */
	xfs_agnumber_t	pag_agno;	/* AG this structure belongs to */
	atomic_t	pag_ref;	/* perag reference count */
	char		pagf_init;	/* this agf's entry is initialized */
	char		pagi_init;	/* this agi's entry is initialized */
	char		pagf_metadata;	/* the agf is preferred to be metadata */
	char		pagi_inodeok;	/* The agi is ok for inodes */
	uint8_t		pagf_levels[XFS_BTNUM_AGF];
					/* # of levels in bno & cnt btree */
	uint32_t	pagf_flcount;	/* count of blocks in freelist */
	xfs_extlen_t	pagf_freeblks;	/* total free blocks */
	xfs_extlen_t	pagf_longest;	/* longest free space */
	uint32_t	pagf_btreeblks;	/* # of blocks held in AGF btrees */
	xfs_agino_t	pagi_freecount;	/* number of free inodes */
	xfs_agino_t	pagi_count;	/* number of allocated inodes */

	/*
	 * Inode allocation search lookup optimisation.
	 * If the pagino matches, the search for new inodes
	 * doesn't need to search the near ones again straight away
	 */
	xfs_agino_t	pagl_pagino;
	xfs_agino_t	pagl_leftrec;
	xfs_agino_t	pagl_rightrec;
	int		pagb_count;	/* pagb slots in use */

	/* Blocks reserved for all kinds of metadata. */
	struct xfs_ag_resv	pag_meta_resv;
	/* Blocks reserved for just AGFL-based metadata. */
	struct xfs_ag_resv	pag_agfl_resv;

	/* reference count */
	uint8_t		pagf_refcount_level;
} xfs_perag_t;

static inline struct xfs_ag_resv *
xfs_perag_resv(
	struct xfs_perag	*pag,
	enum xfs_ag_resv_type	type)
{
	switch (type) {
	case XFS_AG_RESV_METADATA:
		return &pag->pag_meta_resv;
	case XFS_AG_RESV_AGFL:
		return &pag->pag_agfl_resv;
	default:
		return NULL;
	}
}

#define LIBXFS_MOUNT_DEBUGGER		0x0001
#define LIBXFS_MOUNT_32BITINODES	0x0002
#define LIBXFS_MOUNT_32BITINOOPT	0x0004
#define LIBXFS_MOUNT_COMPAT_ATTR	0x0008
#define LIBXFS_MOUNT_ATTR2		0x0010
#define LIBXFS_MOUNT_WANT_CORRUPTED	0x0020

#define LIBXFS_BHASHSIZE(sbp) 		(1<<10)

extern xfs_mount_t	*libxfs_mount (xfs_mount_t *, xfs_sb_t *,
				dev_t, dev_t, dev_t, int);
extern void	libxfs_umount (xfs_mount_t *);
extern void	libxfs_rtmount_destroy (xfs_mount_t *);

#endif	/* __XFS_MOUNT_H__ */
