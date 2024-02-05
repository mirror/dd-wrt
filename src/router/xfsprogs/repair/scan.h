// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#ifndef _XR_SCAN_H
#define _XR_SCAN_H

struct blkmap;

void set_mp(xfs_mount_t *mpp);

int scan_lbtree(
	xfs_fsblock_t	root,
	int		nlevels,
	int		(*func)(struct xfs_btree_block	*block,
				int			level,
				int			type,
				int			whichfork,
				xfs_fsblock_t		bno,
				xfs_ino_t		ino,
				xfs_rfsblock_t		*tot,
				xfs_extnum_t		*nex,
				struct blkmap		**blkmapp,
				bmap_cursor_t		*bm_cursor,
				int			isroot,
				int			check_dups,
				int			*dirty,
				uint64_t		magic),
	int		type,
	int		whichfork,
	xfs_ino_t	ino,
	xfs_rfsblock_t	*tot,
	xfs_extnum_t	*nex,
	struct blkmap	**blkmapp,
	bmap_cursor_t	*bm_cursor,
	int		isroot,
	int		check_dups,
	uint64_t	magic,
	const struct xfs_buf_ops *ops);

int scan_bmapbt(
	struct xfs_btree_block	*block,
	int			level,
	int			type,
	int			whichfork,
	xfs_fsblock_t		bno,
	xfs_ino_t		ino,
	xfs_rfsblock_t		*tot,
	xfs_extnum_t		*nex,
	struct blkmap		**blkmapp,
	bmap_cursor_t		*bm_cursor,
	int			isroot,
	int			check_dups,
	int			*dirty,
	uint64_t		magic);

void
scan_ags(
	struct xfs_mount	*mp,
	int			scan_threads);

#endif /* _XR_SCAN_H */
