/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

#ifndef _XR_DIR2_H
#define	_XR_DIR2_H

struct blkmap;
struct bmap_ext;

int
process_dir2(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dirty,
	char		*dirname,
	xfs_ino_t	*parent,
	struct blkmap	*blkmap);

void
process_sf_dir2_fixi8(
	struct xfs_mount	*mp,
	struct xfs_dir2_sf_hdr	*sfp,
	xfs_dir2_sf_entry_t	**next_sfep);

int
dir2_is_badino(
	xfs_ino_t	ino);

#endif	/* _XR_DIR2_H */
