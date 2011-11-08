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

#ifndef _XR_DIR_H
#define _XR_DIR_H

struct blkmap;

/* 1 bit per byte, max XFS blocksize == 64K bits / NBBY */
#define DA_BMAP_SIZE		8192

typedef unsigned char	da_freemap_t;

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
typedef struct da_level_state  {
	xfs_buf_t	*bp;		/* block bp */
#ifdef XR_DIR_TRACE
	xfs_da_intnode_t *n;		/* bp data */
#endif
	xfs_dablk_t	bno;		/* file block number */
	xfs_dahash_t	hashval;	/* last verified hashval */
	int		index;		/* current index in block */
	int		dirty;		/* is buffer dirty ? (1 == yes) */
} da_level_state_t;

typedef struct da_bt_cursor  {
	int			active;	/* highest level in tree (# levels-1) */
	int			type;	/* 0 if dir, 1 if attr */
	xfs_ino_t		ino;
	xfs_dablk_t		greatest_bno;
	xfs_dinode_t		*dip;
	da_level_state_t	level[XFS_DA_NODE_MAXDEPTH];
	struct blkmap		*blkmap;
} da_bt_cursor_t;


/* ROUTINES */

void
err_release_da_cursor(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level);

xfs_dfsbno_t
get_first_dblock_fsbno(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dino);

void
init_da_freemap(
	da_freemap_t *dir_freemap);

int
namecheck(
	char		*name,
	int		length);

int
process_shortform_dir(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,	/* is dinode buffer dirty? */
	xfs_ino_t	*parent,	/* out - NULLFSINO if entry doesn't exist */
	char		*dirname,	/* directory pathname */
	int		*repair);	/* out - 1 if dir was fixed up */

int
process_dir(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dirty,
	char		*dirname,
	xfs_ino_t	*parent,
	struct blkmap	*blkmap);

void
release_da_cursor(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level);

int
set_da_freemap(
	xfs_mount_t *mp, da_freemap_t *map,
	int start, int stop);

int
traverse_int_dablock(
	xfs_mount_t	*mp,
	da_bt_cursor_t		*da_cursor,
	xfs_dablk_t		*rbno,
	int			whichfork);

int
verify_da_path(
	xfs_mount_t	*mp,
	da_bt_cursor_t		*cursor,
	const int		p_level);

int
verify_final_da_path(
	xfs_mount_t	*mp,
	da_bt_cursor_t		*cursor,
	const int		p_level);


#endif /* _XR_DIR_H */
