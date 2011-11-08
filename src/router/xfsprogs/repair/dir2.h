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

/*
 * the cursor gets passed up and down the da btree processing
 * routines.  The interior block processing routines use the
 * cursor to determine if the pointers to and from the preceding
 * and succeeding sibling blocks are ok and whether the values in
 * the current block are consistent with the entries in the parent
 * nodes.  When a block is traversed, a parent-verification routine
 * is called to verify if the next logical entry in the next level up
 * is consistent with the greatest hashval in the next block of the
 * current level.  The verification routine is itself recursive and
 * calls itself if it has to traverse an interior block to get
 * the next logical entry.  The routine recurses upwards through
 * the tree until it finds a block where it can simply step to
 * the next entry.  The hashval in that entry should be equal to
 * the hashval being passed to it (the greatest hashval in the block
 * that the entry points to).  If that isn't true, then the tree
 * is blown and we need to trash it, salvage and trash it, or fix it.
 * Currently, we just trash it.
 */
typedef struct dir2_level_state  {
	xfs_dabuf_t	*bp;		/* block bp */
	xfs_dablk_t	bno;		/* file block number */
	xfs_dahash_t	hashval;	/* last verified hashval */
	int		index;		/* current index in block */
	int		dirty;		/* is buffer dirty ? (1 == yes) */
} dir2_level_state_t;

typedef struct dir2_bt_cursor  {
	int			active;	/* highest level in tree (# levels-1) */
	int			type;	/* 0 if dir, 1 if attr */
	xfs_ino_t		ino;
	xfs_dablk_t		greatest_bno;
	xfs_dinode_t		*dip;
	dir2_level_state_t	level[XFS_DA_NODE_MAXDEPTH];
	struct blkmap		*blkmap;
} dir2_bt_cursor_t;


/* ROUTINES */

void
err_release_dir2_cursor(
	xfs_mount_t		*mp,
	dir2_bt_cursor_t	*cursor,
	int			prev_level);

xfs_dabuf_t *
da_read_buf(
	xfs_mount_t	*mp,
	int		nex,
	struct bmap_ext	*bmp);

int
da_bwrite(
	xfs_mount_t	*mp,
	xfs_dabuf_t	*bp);

void
da_brelse(
	xfs_dabuf_t	*bp);

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
	xfs_dir2_sf_t		*sfp,
	xfs_dir2_sf_entry_t	**next_sfep);

void
dir2_add_badlist(
	xfs_ino_t	ino);

int
dir2_is_badino(
	xfs_ino_t	ino);

#endif	/* _XR_DIR2_H */
