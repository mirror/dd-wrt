/*
 * Copyright (c) 2015 Red Hat, Inc.
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

#ifndef _XR_DA_UTIL_H
#define	_XR_DA_UTIL_H

struct da_level_state  {
	xfs_buf_t	*bp;		/* block bp */
	xfs_dablk_t	bno;		/* file block number */
	xfs_dahash_t	hashval;	/* last verified hashval */
	int		index;		/* current index in block */
	int		dirty;		/* is buffer dirty ? (1 == yes) */
};

typedef struct da_bt_cursor {
	int			active;	/* highest level in tree (# levels-1) */
	xfs_ino_t		ino;
	xfs_dablk_t		greatest_bno;
	xfs_dinode_t		*dip;
	struct da_level_state	level[XFS_DA_NODE_MAXDEPTH];
	struct blkmap		*blkmap;
} da_bt_cursor_t;

int
namecheck(
	char		*name,
	int		length);

struct xfs_buf *
da_read_buf(
	xfs_mount_t	*mp,
	int		nex,
	bmap_ext_t	*bmp,
	const struct xfs_buf_ops *ops);

void
release_da_cursor(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level);

void
err_release_da_cursor(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level);

int
traverse_int_dablock(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*da_cursor,
	xfs_dablk_t	*rbno,
	int		whichfork);

int
verify_da_path(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	const int	p_level,
	int		whichfork);

int
verify_final_da_path(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	const int	p_level,
	int		whichfork);
#endif	/* _XR_DA_UTIL_H */
