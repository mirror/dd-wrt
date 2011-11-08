/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
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

#include <libxfs.h>
#include "avl.h"
#include "globals.h"
#include "incore.h"
#include "err_protos.h"
#include "dinode.h"
#include "dir.h"
#include "dir2.h"
#include "bmap.h"
#include "prefetch.h"
#include "progress.h"

/*
 * Tag bad directory entries with this.
 * We can't tag them with -1 since that will look like a
 * data_unused_t instead of a data_entry_t.
 */
#define	BADFSINO	((xfs_ino_t)0xfeffffffffffffffULL)

/*
 * Known bad inode list.  These are seen when the leaf and node
 * block linkages are incorrect.
 */
typedef struct dir2_bad {
	xfs_ino_t	ino;
	struct dir2_bad	*next;
} dir2_bad_t;
dir2_bad_t *dir2_bad_list;

void
dir2_add_badlist(
	xfs_ino_t	ino)
{
	dir2_bad_t	*l;

	if ((l = malloc(sizeof(dir2_bad_t))) == NULL) {
		do_error(
_("malloc failed (%zu bytes) dir2_add_badlist:ino %" PRIu64 "\n"),
			sizeof(dir2_bad_t), ino);
		exit(1);
	}
	l->next = dir2_bad_list;
	dir2_bad_list = l;
	l->ino = ino;
}

int
dir2_is_badino(
	xfs_ino_t	ino)
{
	dir2_bad_t	*l;

	for (l = dir2_bad_list; l; l = l->next)
		if (l->ino == ino)
			return 1;
	return 0;
}

/*
 * Multibuffer handling.
 * V2 directory blocks can be noncontiguous, needing multiple buffers.
 */
xfs_dabuf_t *
da_read_buf(
	xfs_mount_t	*mp,
	int		nex,
	bmap_ext_t	*bmp)
{
	xfs_buf_t	*bp;
	xfs_buf_t	*bparray[4];
	xfs_buf_t	**bplist;
	xfs_dabuf_t	*dabuf;
	int		i;
	int		off;

	if (nex > (sizeof(bparray)/sizeof(xfs_buf_t *))) {
		bplist = calloc(nex, sizeof(*bplist));
		if (bplist == NULL) {
			do_error(_("couldn't malloc dir2 buffer list\n"));
			exit(1);
		}
	}
	else {
		/* common case avoids calloc/free */
		bplist = bparray;
	}
	for (i = 0; i < nex; i++) {
		pftrace("about to read off %llu (len = %d)",
			(long long)XFS_FSB_TO_DADDR(mp, bmp[i].startblock),
			XFS_FSB_TO_BB(mp, bmp[i].blockcount));

		bplist[i] = libxfs_readbuf(mp->m_dev,
				XFS_FSB_TO_DADDR(mp, bmp[i].startblock),
				XFS_FSB_TO_BB(mp, bmp[i].blockcount), 0);
		if (!bplist[i]) {
			nex = i;
			goto failed;
		}

		pftrace("readbuf %p (%llu, %d)", bplist[i],
			(long long)XFS_BUF_ADDR(bplist[i]),
			XFS_BUF_COUNT(bplist[i]));
	}
	dabuf = malloc(XFS_DA_BUF_SIZE(nex));
	if (dabuf == NULL) {
		do_error(_("couldn't malloc dir2 buffer header\n"));
		exit(1);
	}
	dabuf->dirty = 0;
	dabuf->nbuf = nex;
	if (nex == 1) {
		bp = bplist[0];
		dabuf->bbcount = (short)BTOBB(XFS_BUF_COUNT(bp));
		dabuf->data = XFS_BUF_PTR(bp);
		dabuf->bps[0] = bp;
	} else {
		for (i = 0, dabuf->bbcount = 0; i < nex; i++) {
			dabuf->bps[i] = bp = bplist[i];
			dabuf->bbcount += BTOBB(XFS_BUF_COUNT(bp));
		}
		dabuf->data = malloc(BBTOB(dabuf->bbcount));
		if (dabuf->data == NULL) {
			do_error(_("couldn't malloc dir2 buffer data\n"));
			exit(1);
		}
		for (i = off = 0; i < nex; i++, off += XFS_BUF_COUNT(bp)) {
			bp = bplist[i];
			memmove((char *)dabuf->data + off, XFS_BUF_PTR(bp),
				XFS_BUF_COUNT(bp));
		}
	}
	if (bplist != bparray)
		free(bplist);
	return dabuf;
failed:
	for (i = 0; i < nex; i++)
		libxfs_putbuf(bplist[i]);
	if (bplist != bparray)
		free(bplist);
	return NULL;
}

static void
da_buf_clean(
	xfs_dabuf_t	*dabuf)
{
	xfs_buf_t	*bp;
	int		i;
	int		off;

	if (dabuf->dirty) {
		dabuf->dirty = 0;
		for (i=off=0; i < dabuf->nbuf; i++, off += XFS_BUF_COUNT(bp)) {
			bp = dabuf->bps[i];
			memmove(XFS_BUF_PTR(bp), (char *)dabuf->data + off,
				XFS_BUF_COUNT(bp));
		}
	}
}

static void
da_buf_done(
	xfs_dabuf_t	*dabuf)
{
	da_buf_clean(dabuf);
	if (dabuf->nbuf > 1)
		free(dabuf->data);
	free(dabuf);
}

int
da_bwrite(
	xfs_mount_t	*mp,
	xfs_dabuf_t	*dabuf)
{
	xfs_buf_t	*bp;
	xfs_buf_t	**bplist;
	int		e;
	int		error;
	int		i;
	int		nbuf;
	int		off;

	if ((nbuf = dabuf->nbuf) == 1) {
		bplist = &bp;
		bp = dabuf->bps[0];
	} else {
		bplist = malloc(nbuf * sizeof(*bplist));
		if (bplist == NULL) {
			do_error(_("couldn't malloc dir2 buffer list\n"));
			exit(1);
		}
		memmove(bplist, dabuf->bps, nbuf * sizeof(*bplist));
		for (i = off = 0; i < nbuf; i++, off += XFS_BUF_COUNT(bp)) {
			bp = bplist[i];
			memmove(XFS_BUF_PTR(bp), (char *)dabuf->data + off,
				XFS_BUF_COUNT(bp));
		}
	}
	da_buf_done(dabuf);
	for (i = error = 0; i < nbuf; i++) {
		e = libxfs_writebuf(bplist[i], 0);
		if (e)
			error = e;
	}
	if (bplist != &bp)
		free(bplist);
	return error;
}

void
da_brelse(
	xfs_dabuf_t	*dabuf)
{
	xfs_buf_t	*bp;
	xfs_buf_t	**bplist;
	int		i;
	int		nbuf;

	if ((nbuf = dabuf->nbuf) == 1) {
		bplist = &bp;
		bp = dabuf->bps[0];
	} else {
		bplist = malloc(nbuf * sizeof(*bplist));
		if (bplist == NULL) {
			do_error(_("couldn't malloc dir2 buffer list\n"));
			exit(1);
		}
		memmove(bplist, dabuf->bps, nbuf * sizeof(*bplist));
	}
	da_buf_done(dabuf);
	for (i = 0; i < nbuf; i++) {
		pftrace("putbuf %p (%llu)", bplist[i],
					(long long)XFS_BUF_ADDR(bplist[i]));
		libxfs_putbuf(bplist[i]);
	}
	if (bplist != &bp)
		free(bplist);
}

/*
 * walk tree from root to the left-most leaf block reading in
 * blocks and setting up cursor.  passes back file block number of the
 * left-most leaf block if successful (bno).  returns 1 if successful,
 * 0 if unsuccessful.
 */
int
traverse_int_dir2block(xfs_mount_t	*mp,
		dir2_bt_cursor_t	*da_cursor,
		xfs_dablk_t		*rbno)
{
	bmap_ext_t		*bmp;
	xfs_dablk_t		bno;
	xfs_dabuf_t		*bp;
	int			i;
	int			nex;
	xfs_da_blkinfo_t	*info;
	xfs_da_intnode_t	*node;
	bmap_ext_t		lbmp;

	/*
	 * traverse down left-side of tree until we hit the
	 * left-most leaf block setting up the btree cursor along
	 * the way.
	 */
	bno = mp->m_dirleafblk;
	i = -1;
	info = NULL;
	da_cursor->active = 0;

	do {
		/*
		 * read in each block along the way and set up cursor
		 */
		nex = blkmap_getn(da_cursor->blkmap, bno, mp->m_dirblkfsbs,
				&bmp, &lbmp);

		if (nex == 0)
			goto error_out;

		bp = da_read_buf(mp, nex, bmp);
		if (bmp != &lbmp)
			free(bmp);
		if (bp == NULL) {
			do_warn(
_("can't read block %u for directory inode %" PRIu64 "\n"),
				bno, da_cursor->ino);
			goto error_out;
		}

		info = bp->data;

		if (be16_to_cpu(info->magic) == XFS_DIR2_LEAFN_MAGIC)  {
			if ( i != -1 ) {
				do_warn(
_("found non-root LEAFN node in inode %" PRIu64 " bno = %u\n"),
					da_cursor->ino, bno);
			}
			*rbno = 0;
			da_brelse(bp);
			return(1);
		} else if (be16_to_cpu(info->magic) != XFS_DA_NODE_MAGIC)  {
			da_brelse(bp);
			do_warn(
_("bad dir magic number 0x%x in inode %" PRIu64 " bno = %u\n"),
				be16_to_cpu(info->magic),
					da_cursor->ino, bno);
			goto error_out;
		}
		node = (xfs_da_intnode_t*)info;
		if (be16_to_cpu(node->hdr.count) > mp->m_dir_node_ents)  {
			da_brelse(bp);
			do_warn(
_("bad record count in inode %" PRIu64 ", count = %d, max = %d\n"), da_cursor->ino,
				be16_to_cpu(node->hdr.count),
				mp->m_dir_node_ents);
			goto error_out;
		}
		/*
		 * maintain level counter
		 */
		if (i == -1) {
			i = da_cursor->active = be16_to_cpu(node->hdr.level);
			if (i >= XFS_DA_NODE_MAXDEPTH) {
				do_warn(
_("bad header depth for directory inode %" PRIu64 "\n"),
					da_cursor->ino);
				da_brelse(bp);
				i = -1;
				goto error_out;
			}
		} else {
			if (be16_to_cpu(node->hdr.level) == i - 1)  {
				i--;
			} else  {
				do_warn(
_("bad directory btree for directory inode %" PRIu64 "\n"),
					da_cursor->ino);
				da_brelse(bp);
				goto error_out;
			}
		}

		da_cursor->level[i].hashval =
					be32_to_cpu(node->btree[0].hashval);
		da_cursor->level[i].bp = bp;
		da_cursor->level[i].bno = bno;
		da_cursor->level[i].index = 0;

		/*
		 * set up new bno for next level down
		 */
		bno = be32_to_cpu(node->btree[0].before);
	} while (info != NULL && i > 1);

	/*
	 * now return block number and get out
	 */
	*rbno = da_cursor->level[0].bno = bno;
	return(1);

error_out:
	while (i > 1 && i <= da_cursor->active)  {
		da_brelse(da_cursor->level[i].bp);
		i++;
	}

	return(0);
}

/*
 * blow out buffer for this level and all the rest above as well
 * if error == 0, we are not expecting to encounter any unreleased
 * buffers (e.g. if we do, it's a mistake).  if error == 1, we're
 * in an error-handling case so unreleased buffers may exist.
 */
void
release_dir2_cursor_int(xfs_mount_t		*mp,
			dir2_bt_cursor_t	*cursor,
			int			prev_level,
			int			error)
{
	int	level = prev_level + 1;

	if (cursor->level[level].bp != NULL)  {
		if (!error)  {
			do_warn(_("release_dir2_cursor_int got unexpected "
				  "non-null bp, dabno = %u\n"),
				cursor->level[level].bno);
		}
		ASSERT(error != 0);

		da_brelse(cursor->level[level].bp);
		cursor->level[level].bp = NULL;
	}

	if (level < cursor->active)
		release_dir2_cursor_int(mp, cursor, level, error);

	return;
}

void
release_dir2_cursor(xfs_mount_t		*mp,
		dir2_bt_cursor_t	*cursor,
		int			prev_level)
{
	release_dir2_cursor_int(mp, cursor, prev_level, 0);
}

void
err_release_dir2_cursor(xfs_mount_t		*mp,
			dir2_bt_cursor_t	*cursor,
			int			prev_level)
{
	release_dir2_cursor_int(mp, cursor, prev_level, 1);
}

/*
 * make sure that all entries in all blocks along the right side of
 * of the tree are used and hashval's are consistent.  level is the
 * level of the descendent block.  returns 0 if good (even if it had
 * to be fixed up), and 1 if bad.  The right edge of the tree is
 * technically a block boundary.  This routine should be used then
 * instead of verify_dir2_path().
 */
int
verify_final_dir2_path(xfs_mount_t	*mp,
		dir2_bt_cursor_t	*cursor,
		const int		p_level)
{
	xfs_da_intnode_t	*node;
	int			bad = 0;
	int			entry;
	int			this_level = p_level + 1;

	/*
	 * the index should point to the next "unprocessed" entry
	 * in the block which should be the final (rightmost) entry
	 */
	entry = cursor->level[this_level].index;
	node = (xfs_da_intnode_t *)(cursor->level[this_level].bp->data);
	/*
	 * check internal block consistency on this level -- ensure
	 * that all entries are used, encountered and expected hashvals
	 * match, etc.
	 */
	if (entry != be16_to_cpu(node->hdr.count) - 1)  {
		do_warn(
		_("directory block used/count inconsistency - %d / %hu\n"),
			entry, be16_to_cpu(node->hdr.count));
		bad++;
	}
	/*
	 * hash values monotonically increasing ???
	 */
	if (cursor->level[this_level].hashval >=
				be32_to_cpu(node->btree[entry].hashval))  {
		do_warn(_("directory/attribute block hashvalue inconsistency, "
			  "expected > %u / saw %u\n"),
			cursor->level[this_level].hashval,
			be32_to_cpu(node->btree[entry].hashval));
		bad++;
	}
	if (be32_to_cpu(node->hdr.info.forw) != 0)  {
		do_warn(_("bad directory/attribute forward block pointer, "
			  "expected 0, saw %u\n"),
			be32_to_cpu(node->hdr.info.forw));
		bad++;
	}
	if (bad)  {
		do_warn(_("bad directory block in inode %" PRIu64 "\n"), cursor->ino);
		return(1);
	}
	/*
	 * keep track of greatest block # -- that gets
	 * us the length of the directory
	 */
	if (cursor->level[this_level].bno > cursor->greatest_bno)
		cursor->greatest_bno = cursor->level[this_level].bno;

	/*
	 * ok, now check descendant block number against this level
	 */
	if (cursor->level[p_level].bno !=
				be32_to_cpu(node->btree[entry].before))
		return(1);

	if (cursor->level[p_level].hashval !=
				be32_to_cpu(node->btree[entry].hashval))  {
		if (!no_modify)  {
			do_warn(
_("correcting bad hashval in non-leaf dir block\n"
  "\tin (level %d) in inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
			node->btree[entry].hashval = cpu_to_be32(
						cursor->level[p_level].hashval);
			cursor->level[this_level].dirty++;
		} else  {
			do_warn(
_("would correct bad hashval in non-leaf dir block\n"
  "\tin (level %d) in inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
		}
	}

	/*
	 * release/write buffer
	 */
	ASSERT(cursor->level[this_level].dirty == 0 ||
		(cursor->level[this_level].dirty && !no_modify));

	if (cursor->level[this_level].dirty && !no_modify)
		da_bwrite(mp, cursor->level[this_level].bp);
	else
		da_brelse(cursor->level[this_level].bp);

	cursor->level[this_level].bp = NULL;

	/*
	 * bail out if this is the root block (top of tree)
	 */
	if (this_level >= cursor->active)
		return(0);
	/*
	 * set hashvalue to correctl reflect the now-validated
	 * last entry in this block and continue upwards validation
	 */
	cursor->level[this_level].hashval =
		be32_to_cpu(node->btree[entry].hashval);

	return(verify_final_dir2_path(mp, cursor, this_level));
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
verify_dir2_path(xfs_mount_t	*mp,
	dir2_bt_cursor_t	*cursor,
	const int		p_level)
{
	xfs_da_intnode_t	*node;
	xfs_da_intnode_t	*newnode;
	xfs_dablk_t		dabno;
	xfs_dabuf_t		*bp;
	int			bad;
	int			entry;
	int			this_level = p_level + 1;
	bmap_ext_t		*bmp;
	int			nex;
	bmap_ext_t		lbmp;

	/*
	 * index is currently set to point to the entry that
	 * should be processed now in this level.
	 */
	entry = cursor->level[this_level].index;
	node = cursor->level[this_level].bp->data;

	/*
	 * if this block is out of entries, validate this
	 * block and move on to the next block.
	 * and update cursor value for said level
	 */
	if (entry >= be16_to_cpu(node->hdr.count))  {
		/*
		 * update the hash value for this level before
		 * validating it.  bno value should be ok since
		 * it was set when the block was first read in.
		 */
		cursor->level[this_level].hashval =
			be32_to_cpu(node->btree[entry - 1].hashval);

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
		if (verify_dir2_path(mp, cursor, this_level))
			return(1);
		/*
		 * ok, now get the next buffer and check sibling pointers
		 */
		dabno = be32_to_cpu(node->hdr.info.forw);
		ASSERT(dabno != 0);
		nex = blkmap_getn(cursor->blkmap, dabno, mp->m_dirblkfsbs,
			&bmp, &lbmp);
		if (nex == 0) {
			do_warn(
_("can't get map info for block %u of directory inode %" PRIu64 "\n"),
				dabno, cursor->ino);
			return(1);
		}

		bp = da_read_buf(mp, nex, bmp);
		if (bmp != &lbmp)
			free(bmp);

		if (bp == NULL) {
			do_warn(
_("can't read block %u for directory inode %" PRIu64 "\n"),
				dabno, cursor->ino);
			return(1);
		}

		newnode = bp->data;
		/*
		 * verify magic number and back pointer, sanity-check
		 * entry count, verify level
		 */
		bad = 0;
		if (XFS_DA_NODE_MAGIC != be16_to_cpu(newnode->hdr.info.magic)) {
			do_warn(
_("bad magic number %x in block %u for directory inode %" PRIu64 "\n"),
				be16_to_cpu(newnode->hdr.info.magic),
				dabno, cursor->ino);
			bad++;
		}
		if (be32_to_cpu(newnode->hdr.info.back) !=
					cursor->level[this_level].bno)  {
			do_warn(
_("bad back pointer in block %u for directory inode %" PRIu64 "\n"),
				dabno, cursor->ino);
			bad++;
		}
		if (be16_to_cpu(newnode->hdr.count) > mp->m_dir_node_ents)  {
			do_warn(
_("entry count %d too large in block %u for directory inode %" PRIu64 "\n"),
				be16_to_cpu(newnode->hdr.count),
				dabno, cursor->ino);
			bad++;
		}
		if (be16_to_cpu(newnode->hdr.level) != this_level)  {
			do_warn(
_("bad level %d in block %u for directory inode %" PRIu64 "\n"),
				be16_to_cpu(newnode->hdr.level),
				dabno, cursor->ino);
			bad++;
		}
		if (bad)  {
			da_brelse(bp);
			return(1);
		}
		/*
		 * update cursor, write out the *current* level if
		 * required.  don't write out the descendant level
		 */
		ASSERT(cursor->level[this_level].dirty == 0 ||
			(cursor->level[this_level].dirty && !no_modify));

		if (cursor->level[this_level].dirty && !no_modify)
			da_bwrite(mp, cursor->level[this_level].bp);
		else
			da_brelse(cursor->level[this_level].bp);
		cursor->level[this_level].bp = bp;
		cursor->level[this_level].dirty = 0;
		cursor->level[this_level].bno = dabno;
		cursor->level[this_level].hashval =
			be32_to_cpu(newnode->btree[0].hashval);
		node = newnode;

		entry = cursor->level[this_level].index = 0;
	}
	/*
	 * ditto for block numbers
	 */
	if (cursor->level[p_level].bno !=
				be32_to_cpu(node->btree[entry].before))
		return(1);
	/*
	 * ok, now validate last hashvalue in the descendant
	 * block against the hashval in the current entry
	 */
	if (cursor->level[p_level].hashval !=
				be32_to_cpu(node->btree[entry].hashval))  {
		if (!no_modify)  {
			do_warn(
_("correcting bad hashval in interior dir block\n"
  "\tin (level %d) in inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
			node->btree[entry].hashval = cpu_to_be32(
					cursor->level[p_level].hashval);
			cursor->level[this_level].dirty++;
		} else  {
			do_warn(
_("would correct bad hashval in interior dir block\n"
  "\tin (level %d) in inode %" PRIu64 ".\n"),
				this_level, cursor->ino);
		}
	}
	/*
	 * increment index for this level to point to next entry
	 * (which should point to the next descendant block)
	 */
	cursor->level[this_level].index++;
	return(0);
}

/*
 * Fix up a shortform directory which was in long form (i8count set)
 * and is now in short form (i8count clear).
 * Return pointer to the end of the data when done.
 */
void
process_sf_dir2_fixi8(
	xfs_dir2_sf_t		*sfp,
	xfs_dir2_sf_entry_t	**next_sfep)
{
	xfs_ino_t		ino;
	xfs_dir2_sf_t		*newsfp;
	xfs_dir2_sf_entry_t	*newsfep;
	xfs_dir2_sf_t		*oldsfp;
	xfs_dir2_sf_entry_t	*oldsfep;
	int			oldsize;

	newsfp = sfp;
	oldsize = (__psint_t)*next_sfep - (__psint_t)sfp;
	oldsfp = malloc(oldsize);
	if (oldsfp == NULL) {
		do_error(_("couldn't malloc dir2 shortform copy\n"));
		exit(1);
	}
	memmove(oldsfp, newsfp, oldsize);
	newsfp->hdr.count = oldsfp->hdr.count;
	newsfp->hdr.i8count = 0;
	ino = xfs_dir2_sf_get_inumber(oldsfp, &oldsfp->hdr.parent);
	xfs_dir2_sf_put_inumber(newsfp, &ino, &newsfp->hdr.parent);
	oldsfep = xfs_dir2_sf_firstentry(oldsfp);
	newsfep = xfs_dir2_sf_firstentry(newsfp);
	while ((int)((char *)oldsfep - (char *)oldsfp) < oldsize) {
		newsfep->namelen = oldsfep->namelen;
		xfs_dir2_sf_put_offset(newsfep,
			xfs_dir2_sf_get_offset(oldsfep));
		memmove(newsfep->name, oldsfep->name, newsfep->namelen);
		ino = xfs_dir2_sf_get_inumber(oldsfp,
			xfs_dir2_sf_inumberp(oldsfep));
		xfs_dir2_sf_put_inumber(newsfp, &ino,
			xfs_dir2_sf_inumberp(newsfep));
		oldsfep = xfs_dir2_sf_nextentry(oldsfp, oldsfep);
		newsfep = xfs_dir2_sf_nextentry(newsfp, newsfep);
	}
	*next_sfep = newsfep;
	free(oldsfp);
}

/*
 * Regenerate legal (minimal) offsets for the shortform directory.
 */
static void
process_sf_dir2_fixoff(
	xfs_dinode_t	*dip)
{
	int			i;
	int			offset;
	xfs_dir2_sf_entry_t	*sfep;
	xfs_dir2_sf_t		*sfp;

	sfp = (xfs_dir2_sf_t *)XFS_DFORK_DPTR(dip);
	sfep = xfs_dir2_sf_firstentry(sfp);
	offset = XFS_DIR2_DATA_FIRST_OFFSET;

	for (i = 0; i < sfp->hdr.count; i++) {
		xfs_dir2_sf_put_offset(sfep, offset);
		offset += xfs_dir2_data_entsize(sfep->namelen);
		sfep = xfs_dir2_sf_nextentry(sfp, sfep);
	}
}

/*
 * this routine performs inode discovery and tries to fix things
 * in place.  available redundancy -- inode data size should match
 * used directory space in inode.
 * a non-zero return value means the directory is bogus and should be blasted.
 */
/* ARGSUSED */
static int
process_sf_dir2(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,	/* out - 1 if dinode buffer dirty */
	char		*dirname,	/* directory pathname */
	xfs_ino_t	*parent,	/* out - NULLFSINO if entry not exist */
	int		*repair)	/* out - 1 if dir was fixed up */
{
	int			bad_offset;
	int			bad_sfnamelen;
	int			i;
	int			i8;
	__int64_t		ino_dir_size;
	int			ino_off;
	ino_tree_node_t		*irec_p;
	int			junkit;
	char			*junkreason = NULL;
	xfs_ino_t		lino;
	int			max_size;
	char			name[MAXNAMELEN + 1];
	int			namelen;
	xfs_dir2_sf_entry_t	*next_sfep;
	int			num_entries;
	int			offset;
	xfs_dir2_sf_t		*sfp;
	xfs_dir2_sf_entry_t	*sfep;
	int			tmp_elen;
	int			tmp_len;
	xfs_dir2_sf_entry_t	*tmp_sfep;
	xfs_ino_t		zero = 0;

	sfp = (xfs_dir2_sf_t *)XFS_DFORK_DPTR(dip);
	max_size = XFS_DFORK_DSIZE(dip, mp);
	num_entries = sfp->hdr.count;
	ino_dir_size = be64_to_cpu(dip->di_size);
	offset = XFS_DIR2_DATA_FIRST_OFFSET;
	bad_offset = *repair = 0;

	ASSERT(ino_dir_size <= max_size);

	/*
	 * Initialize i8 based on size of parent inode number.
	 */
	i8 = (xfs_dir2_sf_get_inumber(sfp, &sfp->hdr.parent)
		> XFS_DIR2_MAX_SHORT_INUM);

	/*
	 * check for bad entry count
	 */
	if (num_entries * xfs_dir2_sf_entsize_byname(sfp, 1) +
		    xfs_dir2_sf_hdr_size(0) > max_size || num_entries == 0)
		num_entries = 0xFF;

	/*
	 * run through entries, stop at first bad entry, don't need
	 * to check for .. since that's encoded in its own field
	 */
	sfep = next_sfep = xfs_dir2_sf_firstentry(sfp);
	for (i = 0;
	     i < num_entries && ino_dir_size > (char *)next_sfep - (char *)sfp;
	     i++) {
		tmp_sfep = NULL;
		sfep = next_sfep;
		junkit = 0;
		bad_sfnamelen = 0;
		lino = xfs_dir2_sf_get_inumber(sfp, xfs_dir2_sf_inumberp(sfep));
		/*
		 * if entry points to self, junk it since only '.' or '..'
		 * should do that and shortform dirs don't contain either
		 * entry.  if inode number is invalid, trash entry.
		 * if entry points to special inodes, trash it.
		 * if inode is unknown but number is valid,
		 * add it to the list of uncertain inodes.  don't
		 * have to worry about an entry pointing to a
		 * deleted lost+found inode because the entry was
		 * deleted at the same time that the inode was cleared.
		 */
		if (lino == ino) {
			junkit = 1;
			junkreason = _("current");
		} else if (verify_inum(mp, lino)) {
			junkit = 1;
			junkreason = _("invalid");
		} else if (lino == mp->m_sb.sb_rbmino)  {
			junkit = 1;
			junkreason = _("realtime bitmap");
		} else if (lino == mp->m_sb.sb_rsumino)  {
			junkit = 1;
			junkreason = _("realtime summary");
		} else if (lino == mp->m_sb.sb_uquotino)  {
			junkit = 1;
			junkreason = _("user quota");
		} else if (lino == mp->m_sb.sb_gquotino)  {
			junkit = 1;
			junkreason = _("group quota");
		} else if ((irec_p = find_inode_rec(mp,
					XFS_INO_TO_AGNO(mp, lino),
					XFS_INO_TO_AGINO(mp, lino))) != NULL) {
			/*
			 * if inode is marked free and we're in inode
			 * discovery mode, leave the entry alone for now.
			 * if the inode turns out to be used, we'll figure
			 * that out when we scan it.  If the inode really
			 * is free, we'll hit this code again in phase 4
			 * after we've finished inode discovery and blow
			 * out the entry then.
			 */
			ino_off = XFS_INO_TO_AGINO(mp, lino) -
				irec_p->ino_startnum;
			ASSERT(is_inode_confirmed(irec_p, ino_off));
			if (is_inode_free(irec_p, ino_off) && !ino_discovery) {
				junkit = 1;
				junkreason = _("free");
			}
		} else if (ino_discovery) {
			/*
			 * put the inode on the uncertain list.  we'll
			 * pull the inode off the list and check it later.
			 * if the inode turns out be bogus, we'll delete
			 * this entry in phase 6.
			 */
			add_inode_uncertain(mp, lino, 0);
		} else  {
			/*
			 * blow the entry out.  we know about all
			 * undiscovered entries now (past inode discovery
			 * phase) so this is clearly a bogus entry.
			 */
			junkit = 1;
			junkreason = _("non-existent");
		}
		namelen = sfep->namelen;
		if (junkit)
			do_warn(
_("entry \"%*.*s\" in shortform directory %" PRIu64 " references %s inode %" PRIu64 "\n"),
				namelen, namelen, sfep->name, ino, junkreason,
				lino);
		if (namelen == 0)  {
			/*
			 * if we're really lucky, this is
			 * the last entry in which case we
			 * can use the dir size to set the
			 * namelen value.  otherwise, forget
			 * it because we're not going to be
			 * able to find the next entry.
			 */
			bad_sfnamelen = 1;

			if (i == num_entries - 1)  {
				namelen = ino_dir_size -
					((__psint_t) &sfep->name[0] -
					 (__psint_t) sfp);
				if (!no_modify)  {
					do_warn(
_("zero length entry in shortform dir %" PRIu64 ", resetting to %d\n"),
						ino, namelen);
					sfep->namelen = namelen;
				} else  {
					do_warn(
_("zero length entry in shortform dir %" PRIu64 ", would set to %d\n"),
						ino, namelen);
				}
			} else  {
				do_warn(
_("zero length entry in shortform dir %" PRIu64 ""),
					ino);
				if (!no_modify)
					do_warn(_(", junking %d entries\n"),
						num_entries - i);
				else
					do_warn(_(", would junk %d entries\n"),
						num_entries - i);
				/*
				 * don't process the rest of the directory,
				 * break out of processing looop
				 */
				break;
			}
		} else if ((__psint_t) sfep - (__psint_t) sfp +
				xfs_dir2_sf_entsize_byentry(sfp, sfep)
							> ino_dir_size)  {
			bad_sfnamelen = 1;

			if (i == num_entries - 1)  {
				namelen = ino_dir_size -
					((__psint_t) &sfep->name[0] -
					 (__psint_t) sfp);
				do_warn(
_("size of last entry overflows space left in in shortform dir %" PRIu64 ", "),
					ino);
				if (!no_modify)  {
					do_warn(_("resetting to %d\n"),
						namelen);
					sfep->namelen = namelen;
					*dino_dirty = 1;
				} else  {
					do_warn(_("would reset to %d\n"),
						namelen);
				}
			} else  {
				do_warn(
_("size of entry #%d overflows space left in in shortform dir %" PRIu64 "\n"),
					i, ino);
				if (!no_modify)  {
					if (i == num_entries - 1)
						do_warn(
						_("junking entry #%d\n"),
							i);
					else
						do_warn(
						_("junking %d entries\n"),
							num_entries - i);
				} else  {
					if (i == num_entries - 1)
						do_warn(
						_("would junk entry #%d\n"),
							i);
					else
						do_warn(
						_("would junk %d entries\n"),
							num_entries - i);
				}

				break;
			}
		}

		/*
		 * check for illegal chars in name.
		 * no need to check for bad length because
		 * the length value is stored in a byte
		 * so it can't be too big, it can only wrap
		 */
		if (namecheck((char *)&sfep->name[0], namelen))  {
			/*
			 * junk entry
			 */
			do_warn(
_("entry contains illegal character in shortform dir %" PRIu64 "\n"),
				ino);
			junkit = 1;
		}

		if (xfs_dir2_sf_get_offset(sfep) < offset) {
			do_warn(
_("entry contains offset out of order in shortform dir %" PRIu64 "\n"),
				ino);
			bad_offset = 1;
		}
		offset = xfs_dir2_sf_get_offset(sfep) +
						xfs_dir2_data_entsize(namelen);

		/*
		 * junk the entry by copying up the rest of the
		 * fork over the current entry and decrementing
		 * the entry count.  if we're in no_modify mode,
		 * just issue the warning instead.  then continue
		 * the loop with the next_sfep pointer set to the
		 * correct place in the fork and other counters
		 * properly set to reflect the deletion if it
		 * happened.
		 */
		if (junkit)  {
			memmove(name, sfep->name, namelen);
			name[namelen] = '\0';

			if (!no_modify)  {
				tmp_elen =
					xfs_dir2_sf_entsize_byentry(sfp, sfep);
				be64_add_cpu(&dip->di_size, -tmp_elen);
				ino_dir_size -= tmp_elen;

				tmp_sfep = (xfs_dir2_sf_entry_t *)
					((__psint_t) sfep + tmp_elen);
				tmp_len = max_size - ((__psint_t) tmp_sfep
							- (__psint_t) sfp);

				memmove(sfep, tmp_sfep, tmp_len);

				sfp->hdr.count -= 1;
				num_entries--;
				memset((void *) ((__psint_t) sfep + tmp_len), 0,
					tmp_elen);

				/*
				 * reset the tmp value to the current
				 * pointer so we'll process the entry
				 * we just moved up
				 */
				tmp_sfep = sfep;

				/*
				 * WARNING:  drop the index i by one
				 * so it matches the decremented count
				 * for accurate comparisons later
				 */
				i--;

				*dino_dirty = 1;
				*repair = 1;

				do_warn(
_("junking entry \"%s\" in directory inode %" PRIu64 "\n"),
					name, ino);
			} else  {
				do_warn(
_("would have junked entry \"%s\" in directory inode %" PRIu64 "\n"),
					name, ino);
			}
		} else if (lino > XFS_DIR2_MAX_SHORT_INUM)
			i8++;
		/*
		 * go onto next entry unless we've just junked an
		 * entry in which the current entry pointer points
		 * to an unprocessed entry.  have to take into zero-len
		 * entries into account in no modify mode since we
		 * calculate size based on next_sfep.
		 */
		next_sfep = (tmp_sfep == NULL)
			? (xfs_dir2_sf_entry_t *) ((__psint_t) sfep
				+ ((!bad_sfnamelen)
					? xfs_dir2_sf_entsize_byentry(sfp,
						sfep)
					: xfs_dir2_sf_entsize_byname(sfp,
						namelen)))
			: tmp_sfep;
	}

	/* sync up sizes and entry counts */

	if (sfp->hdr.count != i) {
		if (no_modify) {
			do_warn(
_("would have corrected entry count in directory %" PRIu64 " from %d to %d\n"),
				ino, sfp->hdr.count, i);
		} else {
			do_warn(
_("corrected entry count in directory %" PRIu64 "u, was %d, now %d\n"),
				ino, sfp->hdr.count, i);
			sfp->hdr.count = i;
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	if (sfp->hdr.i8count != i8)  {
		if (no_modify)  {
			do_warn(
_("would have corrected i8 count in directory %" PRIu64 " from %d to %d\n"),
				ino, sfp->hdr.i8count, i8);
		} else {
			do_warn(
_("corrected i8 count in directory %" PRIu64 ", was %d, now %d\n"),
				ino, sfp->hdr.i8count, i8);
			if (i8 == 0)
				process_sf_dir2_fixi8(sfp, &next_sfep);
			else
				sfp->hdr.i8count = i8;
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	if ((intptr_t)next_sfep - (intptr_t)sfp != ino_dir_size)  {
		if (no_modify)  {
			do_warn(
_("would have corrected directory %" PRIu64 " size from %" PRId64 " to %" PRIdPTR "\n"),
				ino, ino_dir_size,
				(intptr_t)next_sfep - (intptr_t)sfp);
		} else  {
			do_warn(
_("corrected directory %" PRIu64 " size, was %" PRId64 ", now %" PRIdPTR "\n"),
				ino, ino_dir_size,
				(intptr_t)next_sfep - (intptr_t)sfp);

			dip->di_size = cpu_to_be64(
					(__psint_t)next_sfep - (__psint_t)sfp);
			*dino_dirty = 1;
			*repair = 1;
		}
	}
	if (offset + (sfp->hdr.count + 2) * sizeof(xfs_dir2_leaf_entry_t) +
			sizeof(xfs_dir2_block_tail_t) > mp->m_dirblksize) {
		do_warn(_("directory %" PRIu64 " offsets too high\n"), ino);
		bad_offset = 1;
	}
	if (bad_offset) {
		if (no_modify) {
			do_warn(
_("would have corrected entry offsets in directory %" PRIu64 "\n"),
				ino);
		} else {
			do_warn(
_("corrected entry offsets in directory %" PRIu64 "\n"),
				ino);
			process_sf_dir2_fixoff(dip);
			*dino_dirty = 1;
			*repair = 1;
		}
	}

	/*
	 * check parent (..) entry
	 */
	*parent = xfs_dir2_sf_get_inumber(sfp, &sfp->hdr.parent);

	/*
	 * if parent entry is bogus, null it out.  we'll fix it later .
	 */
	if (verify_inum(mp, *parent))  {

		do_warn(
_("bogus .. inode number (%" PRIu64 ") in directory inode %" PRIu64 ", "),
				*parent, ino);
		*parent = NULLFSINO;
		if (!no_modify)  {
			do_warn(_("clearing inode number\n"));

			xfs_dir2_sf_put_inumber(sfp, &zero, &sfp->hdr.parent);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(_("would clear inode number\n"));
		}
	} else if (ino == mp->m_sb.sb_rootino && ino != *parent) {
		/*
		 * root directories must have .. == .
		 */
		if (!no_modify)  {
			do_warn(
_("corrected root directory %" PRIu64 " .. entry, was %" PRIu64 ", now %" PRIu64 "\n"),
				ino, *parent, ino);
			*parent = ino;
			xfs_dir2_sf_put_inumber(sfp, parent, &sfp->hdr.parent);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(
_("would have corrected root directory %" PRIu64 " .. entry from %" PRIu64" to %" PRIu64 "\n"),
				ino, *parent, ino);
		}
	} else if (ino == *parent && ino != mp->m_sb.sb_rootino)  {
		/*
		 * likewise, non-root directories can't have .. pointing
		 * to .
		 */
		*parent = NULLFSINO;
		do_warn(
_("bad .. entry in directory inode %" PRIu64 ", points to self, "),
			ino);
		if (!no_modify)  {
			do_warn(_("clearing inode number\n"));

			xfs_dir2_sf_put_inumber(sfp, &zero, &sfp->hdr.parent);
			*dino_dirty = 1;
			*repair = 1;
		} else  {
			do_warn(_("would clear inode number\n"));
		}
	}

	return(0);
}

/*
 * Process one directory data block.
 */
/* ARGSUSED */
static int
process_dir2_data(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	char		*dirname,	/* directory pathname */
	xfs_ino_t	*parent,	/* out - NULLFSINO if entry not exist */
	xfs_dabuf_t	*bp,
	int		*dot,		/* out - 1 if there is a dot, else 0 */
	int		*dotdot,	/* out - 1 if there's a dotdot, else 0 */
	xfs_dablk_t	da_bno,
	char		*endptr)
{
	int			badbest;
	xfs_dir2_data_free_t	*bf;
	int			clearino;
	char			*clearreason = NULL;
	xfs_dir2_data_t		*d;
	xfs_dir2_data_entry_t	*dep;
	xfs_dir2_data_free_t	*dfp;
	xfs_dir2_data_unused_t	*dup;
	int			freeseen;
	int			i;
	int			ino_off;
	ino_tree_node_t		*irec_p;
	int			junkit;
	int			lastfree;
	int			nm_illegal;
	char			*ptr;
	xfs_ino_t		ent_ino;

	d = bp->data;
	bf = d->hdr.bestfree;
	ptr = (char *)d->u;
	badbest = lastfree = freeseen = 0;
	if (be16_to_cpu(bf[0].length) == 0) {
		badbest |= be16_to_cpu(bf[0].offset) != 0;
		freeseen |= 1 << 0;
	}
	if (be16_to_cpu(bf[1].length) == 0) {
		badbest |= be16_to_cpu(bf[1].offset) != 0;
		freeseen |= 1 << 1;
	}
	if (be16_to_cpu(bf[2].length) == 0) {
		badbest |= be16_to_cpu(bf[2].offset) != 0;
		freeseen |= 1 << 2;
	}
	badbest |= be16_to_cpu(bf[0].length) < be16_to_cpu(bf[1].length);
	badbest |= be16_to_cpu(bf[1].length) < be16_to_cpu(bf[2].length);
	while (ptr < endptr) {
		dup = (xfs_dir2_data_unused_t *)ptr;
		/*
		 * If it's unused, look for the space in the bestfree table.
		 * If we find it, account for that, else make sure it doesn't
		 * need to be there.
		 */
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			if (ptr + be16_to_cpu(dup->length) > endptr ||
			    be16_to_cpu(dup->length) == 0 ||
			    (be16_to_cpu(dup->length) & (XFS_DIR2_DATA_ALIGN - 1)))
				break;
			if (be16_to_cpu(*xfs_dir2_data_unused_tag_p(dup)) !=
							(char *)dup - (char *)d)
				break;
			badbest |= lastfree != 0;
			dfp = xfs_dir2_data_freefind(d, dup);
			if (dfp) {
				i = dfp - bf;
				badbest |= (freeseen & (1 << i)) != 0;
				freeseen |= 1 << i;
			} else
				badbest |= be16_to_cpu(dup->length) >
					be16_to_cpu(bf[2].length);
			ptr += be16_to_cpu(dup->length);
			lastfree = 1;
			continue;
		}
		dep = (xfs_dir2_data_entry_t *)ptr;
		if (ptr + xfs_dir2_data_entsize(dep->namelen) > endptr)
			break;
		if (be16_to_cpu(*xfs_dir2_data_entry_tag_p(dep)) !=
		    				(char *)dep - (char *)d)
			break;
		ptr += xfs_dir2_data_entsize(dep->namelen);
		lastfree = 0;
	}
	/*
	 * Dropped out before we processed everything, give up.
	 * Phase 6 will kill this block if we don't kill the inode.
	 */
	if (ptr != endptr) {
		do_warn(_("corrupt block %u in directory inode %" PRIu64 "\n"),
			da_bno, ino);
		if (!no_modify)
			do_warn(_("\twill junk block\n"));
		else
			do_warn(_("\twould junk block\n"));
		return 1;
	}
	ptr = (char *)d->u;
	/*
	 * Process the entries now.
	 */
	while (ptr < endptr) {
		dup = (xfs_dir2_data_unused_t *)ptr;
		if (be16_to_cpu(dup->freetag) == XFS_DIR2_DATA_FREE_TAG) {
			ptr += be16_to_cpu(dup->length);
			continue;
		}
		dep = (xfs_dir2_data_entry_t *)ptr;
		ent_ino = be64_to_cpu(dep->inumber);
		clearino = 1;
		clearreason = NULL;
		/*
		 * We may have to blow out an entry because of bad inode
		 * numbers.  Do NOT touch the name until after we've computed
		 * the hashvalue and done a namecheck() on the name.
		 *
		 * Conditions must either set clearino to zero or set
		 * clearreason why it's being cleared.
		 */
		if (!ino_discovery && ent_ino == BADFSINO) {
			/*
			 * Don't do a damned thing.  We already found this
			 * (or did it ourselves) during phase 3.
			 */
			clearino = 0;
		} else if (verify_inum(mp, ent_ino)) {
			/*
			 * Bad inode number.  Clear the inode number and the
			 * entry will get removed later.  We don't trash the
			 * directory since it's still structurally intact.
			 */
			clearreason = _("invalid");
		} else if (ent_ino == mp->m_sb.sb_rbmino) {
			clearreason = _("realtime bitmap");
		} else if (ent_ino == mp->m_sb.sb_rsumino) {
			clearreason = _("realtime summary");
		} else if (ent_ino == mp->m_sb.sb_uquotino) {
			clearreason = _("user quota");
		} else if (ent_ino == mp->m_sb.sb_gquotino) {
			clearreason = _("group quota");
		} else {
			irec_p = find_inode_rec(mp,
						XFS_INO_TO_AGNO(mp, ent_ino),
						XFS_INO_TO_AGINO(mp, ent_ino));
			if (irec_p == NULL) {
				if (ino_discovery) {
					add_inode_uncertain(mp, ent_ino, 0);
					clearino = 0;
				} else
					clearreason = _("non-existent");
			} else {
				/*
				 * Inode recs should have only confirmed
				 * inodes in them.
				 */
				ino_off = XFS_INO_TO_AGINO(mp, ent_ino)
							- irec_p->ino_startnum;
				ASSERT(is_inode_confirmed(irec_p, ino_off));
				/*
				 * If inode is marked free and we're in inode
				 * discovery mode, leave the entry alone for
				 * now.  If the inode turns out to be used,
				 * we'll figure that out when we scan it.
				 * If the inode really is free, we'll hit this
				 * code again in phase 4 after we've finished
				 * inode discovery and blow out the entry then.
				 */
				if (!ino_discovery && is_inode_free(irec_p,
								ino_off))
					clearreason = _("free");
				else
					clearino = 0;
			}
		}
		ASSERT((clearino == 0 && clearreason == NULL) ||
			(clearino != 0 && clearreason != NULL));
		if (clearino)
			do_warn(
_("entry \"%*.*s\" at block %d offset %" PRIdPTR " in directory inode %" PRIu64
  " references %s inode %" PRIu64 "\n"),
				dep->namelen, dep->namelen, dep->name,
				da_bno, (intptr_t)ptr - (intptr_t)d, ino,
				clearreason, ent_ino);
		/*
		 * If the name length is 0 (illegal) make it 1 and blast
		 * the entry.
		 */
		if (dep->namelen == 0) {
			do_warn(
_("entry at block %u offset %" PRIdPTR " in directory inode %" PRIu64
  "has 0 namelength\n"),
				da_bno, (intptr_t)ptr - (intptr_t)d, ino);
			if (!no_modify)
				dep->namelen = 1;
			clearino = 1;
		}
		/*
		 * If needed to clear the inode number, do it now.
		 */
		if (clearino) {
			if (!no_modify) {
				do_warn(
_("\tclearing inode number in entry at offset %" PRIdPTR "...\n"),
					(intptr_t)ptr - (intptr_t)d);
				dep->inumber = cpu_to_be64(BADFSINO);
				ent_ino = BADFSINO;
				bp->dirty = 1;
			} else {
				do_warn(
_("\twould clear inode number in entry at offset %" PRIdPTR "...\n"),
					(intptr_t)ptr - (intptr_t)d);
			}
		}
		/*
		 * Only complain about illegal names in phase 3 (when inode
		 * discovery is turned on).  Otherwise, we'd complain a lot
		 * during phase 4.
		 */
		junkit = ent_ino == BADFSINO;
		nm_illegal = namecheck((char *)dep->name, dep->namelen);
		if (ino_discovery && nm_illegal) {
			do_warn(
_("entry at block %u offset %" PRIdPTR " in directory inode %" PRIu64 " has illegal name \"%*.*s\": "),
				da_bno, (intptr_t)ptr - (intptr_t)d, ino,
				dep->namelen, dep->namelen, dep->name);
			junkit = 1;
		}
		/*
		 * Now we can mark entries with BADFSINO's bad.
		 */
		if (!no_modify && ent_ino == BADFSINO) {
			dep->name[0] = '/';
			bp->dirty = 1;
			junkit = 0;
		}
		/*
		 * Special .. entry processing.
		 */
		if (dep->namelen == 2 &&
		    dep->name[0] == '.' && dep->name[1] == '.') {
			if (!*dotdot) {
				(*dotdot)++;
				*parent = ent_ino;
				/*
				 * What if .. == .?  Legal only in the root
				 * inode.  Blow out entry and set parent to
				 * NULLFSINO otherwise.
				 */
				if (ino == ent_ino &&
						ino != mp->m_sb.sb_rootino) {
					*parent = NULLFSINO;
					do_warn(
_("bad .. entry in directory inode %" PRIu64 ", points to self: "),
						ino);
					junkit = 1;
				}
				/*
				 * We have to make sure that . == .. in the
				 * root inode.
				 */
				else if (ino != ent_ino &&
						ino == mp->m_sb.sb_rootino) {
					do_warn(
_("bad .. entry in root directory inode %" PRIu64 ", was %" PRIu64 ": "),
						ino, ent_ino);
					if (!no_modify) {
						do_warn(_("correcting\n"));
						dep->inumber = cpu_to_be64(ino);
						bp->dirty = 1;
					} else {
						do_warn(_("would correct\n"));
					}
				}
			}
			/*
			 * Can't fix the directory unless we know which ..
			 * entry is the right one.  Both have valid inode
			 * numbers or we wouldn't be here.  So since both
			 * seem equally valid, trash this one.
			 */
			else {
				do_warn(
_("multiple .. entries in directory inode %" PRIu64 ": "),
					ino);
				junkit = 1;
			}
		}
		/*
		 * Special . entry processing.
		 */
		else if (dep->namelen == 1 && dep->name[0] == '.') {
			if (!*dot) {
				(*dot)++;
				if (ent_ino != ino) {
					do_warn(
_("bad . entry in directory inode %" PRIu64 ", was %" PRIu64 ": "),
						ino, ent_ino);
					if (!no_modify) {
						do_warn(_("correcting\n"));
						dep->inumber = cpu_to_be64(ino);
						bp->dirty = 1;
					} else {
						do_warn(_("would correct\n"));
					}
				}
			} else {
				do_warn(
_("multiple . entries in directory inode %" PRIu64 ": "),
					ino);
				junkit = 1;
			}
		}
		/*
		 * All other entries -- make sure only . references self.
		 */
		else if (ent_ino == ino) {
			do_warn(
_("entry \"%*.*s\" in directory inode %" PRIu64 " points to self: "),
				dep->namelen, dep->namelen, dep->name, ino);
			junkit = 1;
		}
		/*
		 * Clear junked entries.
		 */
		if (junkit) {
			if (!no_modify) {
				dep->name[0] = '/';
				bp->dirty = 1;
				do_warn(_("clearing entry\n"));
			} else {
				do_warn(_("would clear entry\n"));
			}
		}
		/*
		 * Advance to the next entry.
		 */
		ptr += xfs_dir2_data_entsize(dep->namelen);
	}
	/*
	 * Check the bestfree table.
	 */
	if (freeseen != 7 || badbest) {
		do_warn(
_("bad bestfree table in block %u in directory inode %" PRIu64 ": "),
			da_bno, ino);
		if (!no_modify) {
			do_warn(_("repairing table\n"));
			libxfs_dir2_data_freescan(mp, d, &i);
			bp->dirty = 1;
		} else {
			do_warn(_("would repair table\n"));
		}
	}
	return 0;
}

/*
 * Process a block-format directory.
 */
/* ARGSUSED */
static int
process_block_dir2(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,	/* out - 1 if dinode buffer dirty */
	char		*dirname,	/* directory pathname */
	xfs_ino_t	*parent,	/* out - NULLFSINO if entry not exist */
	blkmap_t	*blkmap,
	int		*dot,		/* out - 1 if there is a dot, else 0 */
	int		*dotdot,	/* out - 1 if there's a dotdot, else 0 */
	int		*repair)	/* out - 1 if something was fixed */
{
	xfs_dir2_block_t	*block;
	xfs_dir2_leaf_entry_t	*blp;
	bmap_ext_t		*bmp;
	xfs_dabuf_t		*bp;
	xfs_dir2_block_tail_t	*btp;
	int			nex;
	int			rval;
	bmap_ext_t		lbmp;

	*repair = *dot = *dotdot = 0;
	*parent = NULLFSINO;
	nex = blkmap_getn(blkmap, mp->m_dirdatablk, mp->m_dirblkfsbs, &bmp, &lbmp);
	if (nex == 0) {
		do_warn(
_("block %u for directory inode %" PRIu64 " is missing\n"),
			mp->m_dirdatablk, ino);
		return 1;
	}
	bp = da_read_buf(mp, nex, bmp);
	if (bmp != &lbmp)
		free(bmp);
	if (bp == NULL) {
		do_warn(
_("can't read block %u for directory inode %" PRIu64 "\n"),
			mp->m_dirdatablk, ino);
		return 1;
	}
	/*
	 * Verify the block
	 */
	block = bp->data;
	if (be32_to_cpu(block->hdr.magic) != XFS_DIR2_BLOCK_MAGIC)
		do_warn(
_("bad directory block magic # %#x in block %u for directory inode %" PRIu64 "\n"),
			be32_to_cpu(block->hdr.magic), mp->m_dirdatablk, ino);
	/*
	 * process the data area
	 * this also checks & fixes the bestfree
	 */
	btp = xfs_dir2_block_tail_p(mp, block);
	blp = xfs_dir2_block_leaf_p(btp);
	/*
	 * Don't let this go past the end of the block.
	 */
	if ((char *)blp > (char *)btp)
		blp = (xfs_dir2_leaf_entry_t *)btp;
	rval = process_dir2_data(mp, ino, dip, ino_discovery, dirname, parent,
		bp, dot, dotdot, mp->m_dirdatablk, (char *)blp);
	if (bp->dirty && !no_modify) {
		*repair = 1;
		da_bwrite(mp, bp);
	} else
		da_brelse(bp);
	return rval;
}

/*
 * Validates leaf contents, node format directories only.
 * magic number and sibling pointers checked by caller.
 * Returns 0 if block is ok, 1 if the block is bad.
 * Looking for: out of order hash values, bad stale counts.
 */
static int
process_leaf_block_dir2(
	xfs_mount_t		*mp,
	xfs_dir2_leaf_t		*leaf,
	xfs_dablk_t		da_bno,
	xfs_ino_t		ino,
	xfs_dahash_t		last_hashval,
	xfs_dahash_t		*next_hashval)
{
	int			i;
	int			stale;

	for (i = stale = 0; i < be16_to_cpu(leaf->hdr.count); i++) {
		if ((char *)&leaf->ents[i] >= (char *)leaf + mp->m_dirblksize) {
			do_warn(
_("bad entry count in block %u of directory inode %" PRIu64 "\n"),
				da_bno, ino);
			return 1;
		}
		if (be32_to_cpu(leaf->ents[i].address) == XFS_DIR2_NULL_DATAPTR)
			stale++;
		else if (be32_to_cpu(leaf->ents[i].hashval) < last_hashval) {
			do_warn(
_("bad hash ordering in block %u of directory inode %" PRIu64 "\n"),
				da_bno, ino);
			return 1;
		}
		*next_hashval = last_hashval =
					be32_to_cpu(leaf->ents[i].hashval);
	}
	if (stale != be16_to_cpu(leaf->hdr.stale)) {
		do_warn(
_("bad stale count in block %u of directory inode %" PRIu64 "\n"),
			da_bno, ino);
		return 1;
	}
	return 0;
}

/*
 * Returns 0 if the directory is ok, 1 if it has to be rebuilt.
 */
static int
process_leaf_level_dir2(
	xfs_mount_t		*mp,
	dir2_bt_cursor_t	*da_cursor,
	int			*repair)
{
	bmap_ext_t		*bmp;
	xfs_dabuf_t		*bp;
	int			buf_dirty;
	xfs_dahash_t		current_hashval;
	xfs_dablk_t		da_bno;
	xfs_dahash_t		greatest_hashval;
	xfs_ino_t		ino;
	xfs_dir2_leaf_t		*leaf;
	int			nex;
	xfs_dablk_t		prev_bno;
	bmap_ext_t		lbmp;

	da_bno = da_cursor->level[0].bno;
	ino = da_cursor->ino;
	prev_bno = 0;
	bmp = NULL;
	current_hashval = 0;
	greatest_hashval = 0;
	buf_dirty = 0;

	do {
		nex = blkmap_getn(da_cursor->blkmap, da_bno, mp->m_dirblkfsbs,
			&bmp, &lbmp);
		/*
		 * Directory code uses 0 as the NULL block pointer since 0
		 * is the root block and no directory block pointer can point
		 * to the root block of the btree.
		 */
		ASSERT(da_bno != 0);

		if (nex == 0) {
			do_warn(
_("can't map block %u for directory inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}
		bp = da_read_buf(mp, nex, bmp);
		if (bmp != &lbmp)
			free(bmp);
		bmp = NULL;
		if (bp == NULL) {
			do_warn(
_("can't read file block %u for directory inode %" PRIu64 "\n"),
				da_bno, ino);
			goto error_out;
		}
		leaf = bp->data;
		/*
		 * Check magic number for leaf directory btree block.
		 */
		if (be16_to_cpu(leaf->hdr.info.magic) !=
		   XFS_DIR2_LEAFN_MAGIC) {
			do_warn(
_("bad directory leaf magic # %#x for directory inode %" PRIu64 " block %u\n"),
				be16_to_cpu(leaf->hdr.info.magic),
				ino, da_bno);
			da_brelse(bp);
			goto error_out;
		}
		buf_dirty = 0;
		/*
		 * For each block, process the block, verify its path,
		 * then get next block.  Update cursor values along the way.
		 */
		if (process_leaf_block_dir2(mp, leaf, da_bno, ino,
				current_hashval, &greatest_hashval)) {
			da_brelse(bp);
			goto error_out;
		}
		/*
		 * Index can be set to hdr.count so match the indices of the
		 * interior blocks -- which at the end of the block will point
		 * to 1 after the final real entry in the block.
		 */
		da_cursor->level[0].hashval = greatest_hashval;
		da_cursor->level[0].bp = bp;
		da_cursor->level[0].bno = da_bno;
		da_cursor->level[0].index =
			be16_to_cpu(leaf->hdr.count);
		da_cursor->level[0].dirty = buf_dirty;

		if (be32_to_cpu(leaf->hdr.info.back) != prev_bno) {
			do_warn(
_("bad sibling back pointer for block %u in directory inode %" PRIu64 "\n"),
				da_bno, ino);
			da_brelse(bp);
			goto error_out;
		}
		prev_bno = da_bno;
		da_bno = be32_to_cpu(leaf->hdr.info.forw);
		if (da_bno != 0) {
			if (verify_dir2_path(mp, da_cursor, 0)) {
				da_brelse(bp);
				goto error_out;
			}
		}
		current_hashval = greatest_hashval;
		ASSERT(buf_dirty == 0 || (buf_dirty && !no_modify));
		if (buf_dirty && !no_modify) {
			*repair = 1;
			da_bwrite(mp, bp);
		} else
			da_brelse(bp);
	} while (da_bno != 0);
	if (verify_final_dir2_path(mp, da_cursor, 0)) {
		/*
		 * Verify the final path up (right-hand-side) if still ok.
		 */
		do_warn(_("bad hash path in directory %" PRIu64 "\n"), ino);
		goto error_out;
	}
	/*
	 * Redundant but just for testing.
	 */
	release_dir2_cursor(mp, da_cursor, 0);
	return 0;

error_out:
	/*
	 * Release all buffers holding interior btree blocks.
	 */
	err_release_dir2_cursor(mp, da_cursor, 0);
	if (bmp && (bmp != &lbmp))
		free(bmp);
	return 1;
}

/*
 * Return 1 if the directory's leaf/node space is corrupted and
 * needs to be rebuilt, 0 if it's ok.
 */
static int
process_node_dir2(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	blkmap_t	*blkmap,
	int		*repair)
{
	xfs_dablk_t		bno;
	dir2_bt_cursor_t	da_cursor;

	/*
	 * Try again -- traverse down left-side of tree until we hit the
	 * left-most leaf block setting up the btree cursor along the way.
	 * Then walk the leaf blocks left-to-right, calling a parent
	 * verification routine each time we traverse a block.
	 */
	memset(&da_cursor, 0, sizeof(da_cursor));
	da_cursor.ino = ino;
	da_cursor.dip = dip;
	da_cursor.blkmap = blkmap;

	/*
	 * Now process interior node.
	 */
	if (traverse_int_dir2block(mp, &da_cursor, &bno) == 0)
		return 1;

	/*
	 * Skip directories with a root marked XFS_DIR2_LEAFN_MAGIC
	 */
	if (bno == 0) {
		release_dir2_cursor(mp, &da_cursor, 0);
		return 0;
	} else {
		/*
		 * Now pass cursor and bno into leaf-block processing routine.
		 * The leaf dir level routine checks the interior paths up to
		 * the root including the final right-most path.
		 */
		return process_leaf_level_dir2(mp, &da_cursor, repair);
	}
}

/*
 * Process leaf and node directories.
 * Process the data blocks then, if it's a node directory, check
 * the consistency of those blocks.
 */
static int
process_leaf_node_dir2(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	char		*dirname,	/* directory pathname */
	xfs_ino_t	*parent,	/* out - NULLFSINO if entry not exist */
	blkmap_t	*blkmap,
	int		*dot,		/* out - 1 if there is a dot, else 0 */
	int		*dotdot,	/* out - 1 if there's a dotdot, else 0 */
	int		*repair,	/* out - 1 if something was fixed */
	int		isnode)		/* node directory not leaf */
{
	bmap_ext_t		*bmp;
	xfs_dabuf_t		*bp;
	xfs_dir2_data_t		*data;
	xfs_dfiloff_t		dbno;
	int			good;
	int			i;
	xfs_dfiloff_t		ndbno;
	int			nex;
	int			t;
	bmap_ext_t		lbmp;

	*repair = *dot = *dotdot = good = 0;
	*parent = NULLFSINO;
	ndbno = NULLDFILOFF;
	while ((dbno = blkmap_next_off(blkmap, ndbno, &t)) < mp->m_dirleafblk) {
		nex = blkmap_getn(blkmap, dbno, mp->m_dirblkfsbs, &bmp, &lbmp);
		ndbno = dbno + mp->m_dirblkfsbs - 1;
		if (nex == 0) {
			do_warn(
_("block %" PRIu64 " for directory inode %" PRIu64 " is missing\n"),
				dbno, ino);
			continue;
		}
		bp = da_read_buf(mp, nex, bmp);
		if (bmp != &lbmp)
			free(bmp);
		if (bp == NULL) {
			do_warn(
_("can't read block %" PRIu64 " for directory inode %" PRIu64 "\n"),
				dbno, ino);
			continue;
		}
		data = bp->data;
		if (be32_to_cpu(data->hdr.magic) != XFS_DIR2_DATA_MAGIC)
			do_warn(
_("bad directory block magic # %#x in block %" PRIu64 " for directory inode %" PRIu64 "\n"),
				be32_to_cpu(data->hdr.magic), dbno, ino);
		i = process_dir2_data(mp, ino, dip, ino_discovery, dirname,
			parent, bp, dot, dotdot, (xfs_dablk_t)dbno,
			(char *)data + mp->m_dirblksize);
		if (i == 0)
			good++;
		if (bp->dirty && !no_modify) {
			*repair = 1;
			da_bwrite(mp, bp);
		} else
			da_brelse(bp);
	}
	if (good == 0)
		return 1;
	if (!isnode)
		return 0;
	if (dir2_is_badino(ino))
		return 0;

	if (process_node_dir2(mp, ino, dip, blkmap, repair))
		dir2_add_badlist(ino);
	return 0;

}

/*
 * Returns 1 if things are bad (directory needs to be junked)
 * and 0 if things are ok.  If ino_discovery is 1, add unknown
 * inodes to uncertain inode list.
 */
int
process_dir2(
	xfs_mount_t	*mp,
	xfs_ino_t	ino,
	xfs_dinode_t	*dip,
	int		ino_discovery,
	int		*dino_dirty,
	char		*dirname,
	xfs_ino_t	*parent,
	blkmap_t	*blkmap)
{
	int		dot;
	int		dotdot;
	xfs_dfiloff_t	last;
	int		repair;
	int		res;

	*parent = NULLFSINO;
	dot = dotdot = 0;
	last = 0;

	/*
	 * branch off depending on the type of inode.  This routine
	 * is only called ONCE so all the subordinate routines will
	 * fix '.' and junk '..' if they're bogus.
	 */
	if (blkmap)
		last = blkmap_last_off(blkmap);
	if (be64_to_cpu(dip->di_size) <= XFS_DFORK_DSIZE(dip, mp) &&
			dip->di_format == XFS_DINODE_FMT_LOCAL) {
		dot = dotdot = 1;
		res = process_sf_dir2(mp, ino, dip, ino_discovery, dino_dirty,
			dirname, parent, &repair);
	} else if (last == mp->m_dirblkfsbs &&
			(dip->di_format == XFS_DINODE_FMT_EXTENTS ||
			dip->di_format == XFS_DINODE_FMT_BTREE)) {
		res = process_block_dir2(mp, ino, dip, ino_discovery,
			dino_dirty, dirname, parent, blkmap, &dot, &dotdot,
			&repair);
	} else if (last >= mp->m_dirleafblk + mp->m_dirblkfsbs &&
			(dip->di_format == XFS_DINODE_FMT_EXTENTS ||
			dip->di_format == XFS_DINODE_FMT_BTREE)) {
		res = process_leaf_node_dir2(mp, ino, dip, ino_discovery,
			dirname, parent, blkmap, &dot, &dotdot, &repair,
			last > mp->m_dirleafblk + mp->m_dirblkfsbs);
	} else {
		do_warn(_("bad size/format for directory %" PRIu64 "\n"), ino);
		return 1;
	}
	/*
	 * bad . entries in all directories will be fixed up in phase 6
	 */
	if (dot == 0) {
		do_warn(_("no . entry for directory %" PRIu64 "\n"), ino);
	}

	/*
	 * shortform dirs always have a .. entry.  .. for all longform
	 * directories will get fixed in phase 6. .. for other shortform
	 * dirs also get fixed there.  .. for a shortform root was
	 * fixed in place since we know what it should be
	 */
	if (dotdot == 0 && ino != mp->m_sb.sb_rootino) {
		do_warn(_("no .. entry for directory %" PRIu64 "\n"), ino);
	} else if (dotdot == 0 && ino == mp->m_sb.sb_rootino) {
		do_warn(_("no .. entry for root directory %" PRIu64 "\n"), ino);
		need_root_dotdot = 1;
	}

	ASSERT((ino != mp->m_sb.sb_rootino && ino != *parent) ||
		(ino == mp->m_sb.sb_rootino &&
			(ino == *parent || need_root_dotdot == 1)));

	return res;
}
