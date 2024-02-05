// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#ifndef _XR_DIR2_H
#define	_XR_DIR2_H

struct blkmap;
struct bmap_ext;

int
process_dir2(
	xfs_mount_t		*mp,
	xfs_ino_t		ino,
	struct xfs_dinode	*dip,
	int			ino_discovery,
	int			*dirty,
	char			*dirname,
	xfs_ino_t		*parent,
	struct blkmap		*blkmap);

void
process_sf_dir2_fixi8(
	struct xfs_mount	*mp,
	struct xfs_dir2_sf_hdr	*sfp,
	xfs_dir2_sf_entry_t	**next_sfep);

bool
dir2_is_badino(
	xfs_ino_t	ino);

#endif	/* _XR_DIR2_H */
