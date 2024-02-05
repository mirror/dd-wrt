// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Red Hat, Inc.
 * All Rights Reserved.
 */

/* Various utilities for repair of directory and attribute metadata */

#include "libxfs.h"
#include "globals.h"
#include "err_protos.h"
#include "bmap.h"
#include "da_util.h"

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

/*
 * Multibuffer handling.
 * V2 directory blocks can be noncontiguous, needing multiple buffers.
 * attr blocks are single blocks; this code handles that as well.
 */
struct xfs_buf *
da_read_buf(
	xfs_mount_t	*mp,
	int		nex,
	bmap_ext_t	*bmp,
	const struct xfs_buf_ops *ops)
{
#define MAP_ARRAY_SZ 4
	struct xfs_buf_map map_array[MAP_ARRAY_SZ];
	struct xfs_buf_map *map;
	struct xfs_buf	*bp;
	int		i;

	if (nex > MAP_ARRAY_SZ) {
		map = calloc(nex, sizeof(*map));
		if (map == NULL) {
			do_error(_("couldn't malloc dir2 buffer list\n"));
			exit(1);
		}
	} else {
		/* common case avoids calloc/free */
		map = map_array;
	}
	for (i = 0; i < nex; i++) {
		map[i].bm_bn = XFS_FSB_TO_DADDR(mp, bmp[i].startblock);
		map[i].bm_len = XFS_FSB_TO_BB(mp, bmp[i].blockcount);
	}
	libxfs_buf_read_map(mp->m_dev, map, nex, LIBXFS_READBUF_SALVAGE,
			&bp, ops);
	if (map != map_array)
		free(map);
	return bp;
}

#define FORKNAME(type) (type == XFS_DATA_FORK ? _("directory") : _("attribute"))

/*
 * walk tree from root to the left-most leaf block reading in
 * blocks and setting up cursor.  passes back file block number of the
 * left-most leaf block if successful (bno).  returns 1 if successful,
 * 0 if unsuccessful.
 */
int
traverse_int_dablock(
	xfs_mount_t		*mp,
	da_bt_cursor_t		*da_cursor,
	xfs_dablk_t		*rbno,
	int			whichfork)
{
	bmap_ext_t		*bmp;
	xfs_dablk_t		bno;
	struct xfs_buf		*bp;
	int			i;
	int			nex;
	xfs_da_intnode_t	*node;
	bmap_ext_t		lbmp;
	struct xfs_da_geometry	*geo;
	struct xfs_da3_icnode_hdr nodehdr;

	if (whichfork == XFS_DATA_FORK) {
		geo = mp->m_dir_geo;
		bno = geo->leafblk;
	} else {
		geo = mp->m_attr_geo;
		bno = 0;
	}

	/*
	 * traverse down left-side of tree until we hit the
	 * left-most leaf block setting up the btree cursor along
	 * the way.
	 */
	i = -1;
	node = NULL;
	da_cursor->active = 0;

	do {
		/*
		 * read in each block along the way and set up cursor
		 */
		nex = blkmap_getn(da_cursor->blkmap, bno,
				geo->fsbcount, &bmp, &lbmp);

		if (nex == 0)
			goto error_out;

		bp = da_read_buf(mp, nex, bmp, &xfs_da3_node_buf_ops);
		if (bmp != &lbmp)
			free(bmp);

		if (!bp) {
			do_warn(
_("can't read %s block %u for inode %" PRIu64 "\n"),
				FORKNAME(whichfork), bno, da_cursor->ino);
			goto error_out;
		}

		/* corrupt leafn/node; rebuild the dir. */
		if (bp->b_error == -EFSBADCRC || bp->b_error == -EFSCORRUPTED) {
			do_warn(
_("corrupt %s tree block %u for inode %" PRIu64 "\n"),
				FORKNAME(whichfork), bno, da_cursor->ino);
			libxfs_buf_relse(bp);
			goto error_out;
		}

		node = bp->b_addr;
		libxfs_da3_node_hdr_from_disk(mp, &nodehdr, node);

		if (whichfork == XFS_DATA_FORK &&
		    (nodehdr.magic == XFS_DIR2_LEAFN_MAGIC ||
		    nodehdr.magic == XFS_DIR3_LEAFN_MAGIC)) {
			if (i != -1) {
				do_warn(
_("found non-root LEAFN node in inode %" PRIu64 " bno = %u\n"),
					da_cursor->ino, bno);
			}
			*rbno = 0;
			libxfs_buf_relse(bp);
			return 1;
		}

		if (nodehdr.magic != XFS_DA_NODE_MAGIC &&
		    nodehdr.magic != XFS_DA3_NODE_MAGIC) {
			do_warn(
_("bad %s magic number 0x%x in inode %" PRIu64 " bno = %u\n"),
					FORKNAME(whichfork), nodehdr.magic,
					da_cursor->ino, bno);
			libxfs_buf_relse(bp);
			goto error_out;
		}

		if (nodehdr.count > geo->node_ents) {
			do_warn(
_("bad %s record count in inode %" PRIu64 ", count = %d, max = %d\n"),
				FORKNAME(whichfork), da_cursor->ino,
				nodehdr.count, geo->node_ents);
			libxfs_buf_relse(bp);
			goto error_out;
		}

		/*
		 * maintain level counter
		 */
		if (i == -1) {
			i = da_cursor->active = nodehdr.level;
			if (i < 1 || i >= XFS_DA_NODE_MAXDEPTH) {
				do_warn(
_("bad header depth for directory inode %" PRIu64 "\n"),
					da_cursor->ino);
				libxfs_buf_relse(bp);
				i = -1;
				goto error_out;
			}
		} else {
			if (nodehdr.level == i - 1) {
				i--;
			} else {
				do_warn(
_("bad %s btree for inode %" PRIu64 "\n"),
					FORKNAME(whichfork), da_cursor->ino);
				libxfs_buf_relse(bp);
				goto error_out;
			}
		}

		da_cursor->level[i].hashval =
			be32_to_cpu(nodehdr.btree[0].hashval);
		da_cursor->level[i].bp = bp;
		da_cursor->level[i].bno = bno;
		da_cursor->level[i].index = 0;

		/*
		 * set up new bno for next level down
		 */
		bno = be32_to_cpu(nodehdr.btree[0].before);
	} while (node != NULL && i > 1);

	/*
	 * now return block number and get out
	 */
	*rbno = da_cursor->level[0].bno = bno;
	return 1;

error_out:
	while (i > 1 && i <= da_cursor->active) {
		libxfs_buf_relse(da_cursor->level[i].bp);
		i++;
	}

	return 0;
}

/*
 * blow out buffer for this level and all the rest above as well
 * if error == 0, we are not expecting to encounter any unreleased
 * buffers (e.g. if we do, it's a mistake).  if error == 1, we're
 * in an error-handling case so unreleased buffers may exist.
 */
static void
release_da_cursor_int(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level,
	int		error)
{
	int		level = prev_level + 1;

	if (cursor->level[level].bp != NULL)  {
		if (!error)  {
			do_warn(_("release_da_cursor_int got unexpected "
				  "non-null bp, dabno = %u\n"),
				cursor->level[level].bno);
		}
		ASSERT(error != 0);

		libxfs_buf_relse(cursor->level[level].bp);
		cursor->level[level].bp = NULL;
	}

	if (level < cursor->active)
		release_da_cursor_int(mp, cursor, level, error);

	return;
}

void
release_da_cursor(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level)
{
	release_da_cursor_int(mp, cursor, prev_level, 0);
}

void
err_release_da_cursor(
	xfs_mount_t	*mp,
	da_bt_cursor_t	*cursor,
	int		prev_level)
{
	release_da_cursor_int(mp, cursor, prev_level, 1);
}

/*
 * make sure that all entries in all blocks along the right side of
 * of the tree are used and hashval's are consistent.  level is the
 * level of the descendent block.  returns 0 if good (even if it had
 * to be fixed up), and 1 if bad.  The right edge of the tree is
 * technically a block boundary.  This routine should be used then
 * instead of verify_da_path().
 */
int
verify_final_da_path(
	xfs_mount_t		*mp,
	da_bt_cursor_t		*cursor,
	const int		p_level,
	int			whichfork)
{
	xfs_da_intnode_t	*node;
	xfs_dahash_t		hashval;
	int			bad = 0;
	int			entry;
	int			this_level = p_level + 1;
	struct xfs_da3_icnode_hdr nodehdr;

#ifdef XR_DIR_TRACE
	fprintf(stderr, "in verify_final_da_path, this_level = %d\n",
		this_level);
#endif

	/*
	 * the index should point to the next "unprocessed" entry
	 * in the block which should be the final (rightmost) entry
	 */
	entry = cursor->level[this_level].index;
	node = cursor->level[this_level].bp->b_addr;
	libxfs_da3_node_hdr_from_disk(mp, &nodehdr, node);

	/*
	 * check internal block consistency on this level -- ensure
	 * that all entries are used, encountered and expected hashvals
	 * match, etc.
	 */
	if (entry != nodehdr.count - 1) {
		do_warn(
_("%s block used/count inconsistency - %d/%hu\n"),
			FORKNAME(whichfork), entry, nodehdr.count);
		bad++;
	}
	/*
	 * hash values monotonically increasing ???
	 */
	if (cursor->level[this_level].hashval >
				be32_to_cpu(nodehdr.btree[entry].hashval)) {
		do_warn(
_("%s block hashvalue inconsistency, expected > %u / saw %u\n"),
			FORKNAME(whichfork),
			cursor->level[this_level].hashval,
			be32_to_cpu(nodehdr.btree[entry].hashval));
		bad++;
	}
	if (nodehdr.forw != 0) {
		do_warn(
_("bad %s forward block pointer, expected 0, saw %u\n"),
			FORKNAME(whichfork), nodehdr.forw);
		bad++;
	}
	if (bad) {
		do_warn(_("bad %s block in inode %" PRIu64 "\n"),
			FORKNAME(whichfork), cursor->ino);
		return 1;
	}
	/*
	 * keep track of greatest block # -- that gets
	 * us the length of the directory/attribute
	 */
	if (cursor->level[this_level].bno > cursor->greatest_bno)
		cursor->greatest_bno = cursor->level[this_level].bno;

	/*
	 * ok, now check descendant block number against this level
	 */
	if (cursor->level[p_level].bno !=
	    be32_to_cpu(nodehdr.btree[entry].before)) {
#ifdef XR_DIR_TRACE
		fprintf(stderr, "bad %s btree pointer, child bno should "
				"be %d, block bno is %d, hashval is %u\n",
			FORKNAME(whichfork),
			be16_to_cpu(nodehdr.btree[entry].before),
			cursor->level[p_level].bno,
			cursor->level[p_level].hashval);
		fprintf(stderr, "verify_final_da_path returns 1 (bad) #1a\n");
#endif
		return 1;
	}

	if (cursor->level[p_level].hashval !=
				be32_to_cpu(nodehdr.btree[entry].hashval)) {
		if (!no_modify) {
			do_warn(
_("correcting bad hashval in non-leaf %s block\n"
 "\tin (level %d) in inode %" PRIu64 ".\n"),
				FORKNAME(whichfork), this_level, cursor->ino);
			nodehdr.btree[entry].hashval = cpu_to_be32(
						cursor->level[p_level].hashval);
			cursor->level[this_level].dirty++;
		} else {
			do_warn(
_("would correct bad hashval in non-leaf %s block\n"
 "\tin (level %d) in inode %" PRIu64 ".\n"),
				FORKNAME(whichfork), this_level, cursor->ino);
		}
	}

	/*
	 * Note: squirrel hashval away _before_ releasing the
	 * buffer, preventing a use-after-free problem.
	 */
	hashval = be32_to_cpu(nodehdr.btree[entry].hashval);

	/*
	 * release/write buffer
	 */
	ASSERT(cursor->level[this_level].dirty == 0 ||
		(cursor->level[this_level].dirty && !no_modify));

	if (cursor->level[this_level].dirty && !no_modify) {
		libxfs_buf_mark_dirty(cursor->level[this_level].bp);
		libxfs_buf_relse(cursor->level[this_level].bp);
	}
	else
		libxfs_buf_relse(cursor->level[this_level].bp);

	cursor->level[this_level].bp = NULL;

	/*
	 * bail out if this is the root block (top of tree)
	 */
	if (this_level >= cursor->active) {
#ifdef XR_DIR_TRACE
		fprintf(stderr, "verify_final_da_path returns 0 (ok)\n");
#endif
		return 0;
	}
	/*
	 * set hashvalue to correctly reflect the now-validated
	 * last entry in this block and continue upwards validation
	 */
	cursor->level[this_level].hashval = hashval;

	return verify_final_da_path(mp, cursor, this_level, whichfork);
}

/*
 * Verifies the path from a descendant block up to the root.
 * Should be called when the descendant level traversal hits
 * a block boundary before crossing the boundary (reading in a new
 * block).
 *
 * the directory/attr btrees work differently to the other fs btrees.
 * each interior block contains records that are <hashval, bno>
 * pairs.  The bno is a file bno, not a filesystem bno.  The last
 * hashvalue in the block <bno> will be <hashval>.  BUT unlike
 * the freespace btrees, the *last* value in each block gets
 * propagated up the tree instead of the first value in each block.
 * that is, the interior records point to child blocks and the *greatest*
 * hash value contained by the child block is the one the block above
 * uses as the key for the child block.
 *
 * level is the level of the descendent block.  returns 0 if good,
 * and 1 if bad.  The descendant block may be a leaf block.
 *
 * the invariant here is that the values in the cursor for the
 * levels beneath this level (this_level) and the cursor index
 * for this level *must* be valid.
 *
 * that is, the hashval/bno info is accurate for all
 * DESCENDANTS and match what the node[index] information
 * for the current index in the cursor for this level.
 *
 * the index values in the cursor for the descendant level
 * are allowed to be off by one as they will reflect the
 * next entry at those levels to be processed.
 *
 * the hashvalue for the current level can't be set until
 * we hit the last entry in the block so, it's garbage
 * until set by this routine.
 *
 * bno and bp for the current block/level are always valid
 * since they have to be set so we can get a buffer for the
 * block.
 */
int
verify_da_path(
	xfs_mount_t		*mp,
	da_bt_cursor_t		*cursor,
	const int		p_level,
	int			whichfork)
{
	xfs_da_intnode_t	*node;
	xfs_da_intnode_t	*newnode;
	xfs_dablk_t		dabno;
	struct xfs_buf		*bp;
	int			bad;
	int			entry;
	int			this_level = p_level + 1;
	bmap_ext_t		*bmp;
	int			nex;
	bmap_ext_t		lbmp;
	struct xfs_da_geometry	*geo;
	struct xfs_da3_icnode_hdr nodehdr;

	if (whichfork == XFS_DATA_FORK)
		geo = mp->m_dir_geo;
	else
		geo = mp->m_attr_geo;

	/* No buffer at this level, tree is corrupt. */
	if (cursor->level[this_level].bp == NULL)
		return 1;

	/*
	 * index is currently set to point to the entry that
	 * should be processed now in this level.
	 */
	entry = cursor->level[this_level].index;
	node = cursor->level[this_level].bp->b_addr;
	libxfs_da3_node_hdr_from_disk(mp, &nodehdr, node);

	/* No entries in this node?  Tree is corrupt. */
	if (nodehdr.count == 0)
		return 1;

	/*
	 * if this block is out of entries, validate this
	 * block and move on to the next block.
	 * and update cursor value for said level
	 */
	if (entry >= nodehdr.count) {
		/*
		 * update the hash value for this level before
		 * validating it.  bno value should be ok since
		 * it was set when the block was first read in.
		 */
		cursor->level[this_level].hashval =
				be32_to_cpu(nodehdr.btree[entry - 1].hashval);

		/*
		 * keep track of greatest block # -- that gets
		 * us the length of the directory
		 */
		if (cursor->level[this_level].bno > cursor->greatest_bno)
			cursor->greatest_bno = cursor->level[this_level].bno;

		/*
		 * validate the path for the current used-up block
		 * before we trash it
		 */
		if (verify_da_path(mp, cursor, this_level, whichfork))
			return 1;
		/*
		 * ok, now get the next buffer and check sibling pointers
		 */
		dabno = nodehdr.forw;
		ASSERT(dabno != 0);
		nex = blkmap_getn(cursor->blkmap, dabno, geo->fsbcount,
			&bmp, &lbmp);
		if (nex == 0) {
			do_warn(
_("can't get map info for %s block %u of inode %" PRIu64 "\n"),
				FORKNAME(whichfork), dabno, cursor->ino);
			return 1;
		}

		bp = da_read_buf(mp, nex, bmp, &xfs_da3_node_buf_ops);
		if (bmp != &lbmp)
			free(bmp);

		if (!bp) {
			do_warn(
_("can't read %s block %u for inode %" PRIu64 "\n"),
				FORKNAME(whichfork), dabno, cursor->ino);
			return 1;
		}
		if (bp->b_error == -EFSCORRUPTED || bp->b_error == -EFSBADCRC) {
			do_warn(
_("corrupt %s tree block %u for inode %" PRIu64 "\n"),
				FORKNAME(whichfork), dabno, cursor->ino);
			libxfs_buf_relse(bp);
			return 1;
		}

		newnode = bp->b_addr;
		libxfs_da3_node_hdr_from_disk(mp, &nodehdr, newnode);

		/*
		 * verify magic number and back pointer, sanity-check
		 * entry count, verify level
		 */
		bad = 0;
		if (nodehdr.magic != XFS_DA_NODE_MAGIC &&
		    nodehdr.magic != XFS_DA3_NODE_MAGIC) {
			do_warn(
_("bad magic number %x in %s block %u for inode %" PRIu64 "\n"),
				nodehdr.magic, FORKNAME(whichfork),
				dabno, cursor->ino);
			bad++;
		}
		if (nodehdr.back != cursor->level[this_level].bno) {
			do_warn(
_("bad back pointer in %s block %u for inode %" PRIu64 "\n"),
				FORKNAME(whichfork), dabno, cursor->ino);
			bad++;
		}
		if (nodehdr.count > geo->node_ents) {
			do_warn(
_("entry count %d too large in %s block %u for inode %" PRIu64 "\n"),
				nodehdr.count, FORKNAME(whichfork),
				dabno, cursor->ino);
			bad++;
		}
		if (nodehdr.level != this_level) {
			do_warn(
_("bad level %d in %s block %u for inode %" PRIu64 "\n"),
				nodehdr.level, FORKNAME(whichfork),
				dabno, cursor->ino);
			bad++;
		}
		if (bad) {
#ifdef XR_DIR_TRACE
			fprintf(stderr, "verify_da_path returns 1 (bad) #4\n");
#endif
			libxfs_buf_relse(bp);
			return 1;
		}

		/*
		 * update cursor, write out the *current* level if
		 * required.  don't write out the descendant level
		 */
		ASSERT(cursor->level[this_level].dirty == 0 ||
			(cursor->level[this_level].dirty && !no_modify));

		/*
		 * If block looks ok but CRC didn't match, make sure to
		 * recompute it.
		 */
		if (!no_modify &&
		    cursor->level[this_level].bp->b_error == -EFSBADCRC)
			cursor->level[this_level].dirty = 1;

		if (cursor->level[this_level].dirty && !no_modify) {
			libxfs_buf_mark_dirty(cursor->level[this_level].bp);
			libxfs_buf_relse(cursor->level[this_level].bp);
		}
		else
			libxfs_buf_relse(cursor->level[this_level].bp);

		/* switch cursor to point at the new buffer we just read */
		cursor->level[this_level].bp = bp;
		cursor->level[this_level].dirty = 0;
		cursor->level[this_level].bno = dabno;
		cursor->level[this_level].hashval =
					be32_to_cpu(nodehdr.btree[0].hashval);

		entry = cursor->level[this_level].index = 0;
	}
	/*
	 * ditto for block numbers
	 */
	if (cursor->level[p_level].bno !=
	    be32_to_cpu(nodehdr.btree[entry].before)) {
#ifdef XR_DIR_TRACE
		fprintf(stderr, "bad %s btree pointer, child bno "
			"should be %d, block bno is %d, hashval is %u\n",
			FORKNAME(whichfork),
			be32_to_cpu(nodehdr.btree[entry].before),
			cursor->level[p_level].bno,
			cursor->level[p_level].hashval);
		fprintf(stderr, "verify_da_path returns 1 (bad) #1a\n");
#endif
		return 1;
	}
	/*
	 * ok, now validate last hashvalue in the descendant
	 * block against the hashval in the current entry
	 */
	if (cursor->level[p_level].hashval !=
				be32_to_cpu(nodehdr.btree[entry].hashval)) {
		if (!no_modify) {
			do_warn(
_("correcting bad hashval in interior %s block\n"
  "\tin (level %d) in inode %" PRIu64 ".\n"),
				FORKNAME(whichfork), this_level, cursor->ino);
			nodehdr.btree[entry].hashval = cpu_to_be32(
						cursor->level[p_level].hashval);
			cursor->level[this_level].dirty++;
		} else {
			do_warn(
_("would correct bad hashval in interior %s block\n"
  "\tin (level %d) in inode %" PRIu64 ".\n"),
				FORKNAME(whichfork), this_level, cursor->ino);
		}
	}
	/*
	 * increment index for this level to point to next entry
	 * (which should point to the next descendant block)
	 */
	cursor->level[this_level].index++;
#ifdef XR_DIR_TRACE
	fprintf(stderr, "verify_da_path returns 0 (ok)\n");
#endif
	return 0;
}
