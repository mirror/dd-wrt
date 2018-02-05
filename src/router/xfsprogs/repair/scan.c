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

#include "libxfs.h"
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "scan.h"
#include "versions.h"
#include "bmap.h"
#include "progress.h"
#include "threads.h"
#include "slab.h"
#include "rmap.h"

static xfs_mount_t	*mp = NULL;

/*
 * Variables to validate AG header values against the manual count
 * from the btree traversal.
 */
struct aghdr_cnts {
	xfs_agnumber_t	agno;
	xfs_extlen_t	agffreeblks;
	xfs_extlen_t	agflongest;
	uint64_t	agfbtreeblks;
	uint32_t	agicount;
	uint32_t	agifreecount;
	uint64_t	fdblocks;
	uint64_t	usedblocks;
	uint64_t	ifreecount;
	uint32_t	fibtfreecount;
};

void
set_mp(xfs_mount_t *mpp)
{
	libxfs_bcache_purge();
	mp = mpp;
}

static void
scan_sbtree(
	xfs_agblock_t	root,
	int		nlevels,
	xfs_agnumber_t	agno,
	int		suspect,
	void		(*func)(struct xfs_btree_block	*block,
				int			level,
				xfs_agblock_t		bno,
				xfs_agnumber_t		agno,
				int			suspect,
				int			isroot,
				uint32_t		magic,
				void			*priv),
	int		isroot,
	uint32_t	magic,
	void		*priv,
	const struct xfs_buf_ops *ops)
{
	xfs_buf_t	*bp;

	bp = libxfs_readbuf(mp->m_dev, XFS_AGB_TO_DADDR(mp, agno, root),
			XFS_FSB_TO_BB(mp, 1), 0, ops);
	if (!bp) {
		do_error(_("can't read btree block %d/%d\n"), agno, root);
		return;
	}
	if (bp->b_error == -EFSBADCRC || bp->b_error == -EFSCORRUPTED) {
		do_warn(_("btree block %d/%d is suspect, error %d\n"),
			agno, root, bp->b_error);
		suspect = 1;
	}

	(*func)(XFS_BUF_TO_BLOCK(bp), nlevels - 1, root, agno, suspect,
							isroot, magic, priv);
	libxfs_putbuf(bp);
}

/*
 * returns 1 on bad news (inode needs to be cleared), 0 on good
 */
int
scan_lbtree(
	xfs_fsblock_t	root,
	int		nlevels,
	int		(*func)(struct xfs_btree_block	*block,
				int			level,
				int			type,
				int			whichfork,
				xfs_fsblock_t		bno,
				xfs_ino_t		ino,
				xfs_rfsblock_t		*tot,
				uint64_t		*nex,
				blkmap_t		**blkmapp,
				bmap_cursor_t		*bm_cursor,
				int			isroot,
				int			check_dups,
				int			*dirty,
				uint64_t		magic),
	int		type,
	int		whichfork,
	xfs_ino_t	ino,
	xfs_rfsblock_t	*tot,
	uint64_t	*nex,
	blkmap_t	**blkmapp,
	bmap_cursor_t	*bm_cursor,
	int		isroot,
	int		check_dups,
	uint64_t	magic,
	const struct xfs_buf_ops *ops)
{
	xfs_buf_t	*bp;
	int		err;
	int		dirty = 0;
	bool		badcrc = false;

	bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, root),
		      XFS_FSB_TO_BB(mp, 1), 0, ops);
	if (!bp)  {
		do_error(_("can't read btree block %d/%d\n"),
			XFS_FSB_TO_AGNO(mp, root),
			XFS_FSB_TO_AGBNO(mp, root));
		return(1);
	}

	/*
	 * only check for bad CRC here - caller will determine if there
	 * is a corruption or not and whether it got corrected and so needs
	 * writing back. CRC errors always imply we need to write the block.
	 */
	if (bp->b_error == -EFSBADCRC) {
		do_warn(_("btree block %d/%d is suspect, error %d\n"),
			XFS_FSB_TO_AGNO(mp, root),
			XFS_FSB_TO_AGBNO(mp, root), bp->b_error);
		badcrc = true;
	}

	err = (*func)(XFS_BUF_TO_BLOCK(bp), nlevels - 1,
			type, whichfork, root, ino, tot, nex, blkmapp,
			bm_cursor, isroot, check_dups, &dirty,
			magic);

	ASSERT(dirty == 0 || (dirty && !no_modify));

	if ((dirty || badcrc) && !no_modify)
		libxfs_writebuf(bp, 0);
	else
		libxfs_putbuf(bp);

	return(err);
}

int
scan_bmapbt(
	struct xfs_btree_block	*block,
	int			level,
	int			type,
	int			whichfork,
	xfs_fsblock_t		bno,
	xfs_ino_t		ino,
	xfs_rfsblock_t		*tot,
	uint64_t		*nex,
	blkmap_t		**blkmapp,
	bmap_cursor_t		*bm_cursor,
	int			isroot,
	int			check_dups,
	int			*dirty,
	uint64_t		magic)
{
	int			i;
	int			err;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmbt_key_t		*pkey;
	xfs_bmbt_rec_t		*rp;
	xfs_fileoff_t		first_key;
	xfs_fileoff_t		last_key;
	char			*forkname = get_forkname(whichfork);
	int			numrecs;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	int			state;
	int			error;

	/*
	 * unlike the ag freeblock btrees, if anything looks wrong
	 * in an inode bmap tree, just bail.  it's possible that
	 * we'll miss a case where the to-be-toasted inode and
	 * another inode are claiming the same block but that's
	 * highly unlikely.
	 */
	if (be32_to_cpu(block->bb_magic) != magic) {
		do_warn(
_("bad magic # %#x in inode %" PRIu64 " (%s fork) bmbt block %" PRIu64 "\n"),
			be32_to_cpu(block->bb_magic), ino, forkname, bno);
		return(1);
	}
	if (be16_to_cpu(block->bb_level) != level) {
		do_warn(
_("expected level %d got %d in inode %" PRIu64 ", (%s fork) bmbt block %" PRIu64 "\n"),
			level, be16_to_cpu(block->bb_level),
			ino, forkname, bno);
		return(1);
	}

	if (magic == XFS_BMAP_CRC_MAGIC) {
		/* verify owner */
		if (be64_to_cpu(block->bb_u.l.bb_owner) != ino) {
			do_warn(
_("expected owner inode %" PRIu64 ", got %llu, bmbt block %" PRIu64 "\n"),
				ino,
				(unsigned long long)be64_to_cpu(block->bb_u.l.bb_owner),
				bno);
			return 1;
		}
		/* verify block number */
		if (be64_to_cpu(block->bb_u.l.bb_blkno) !=
		    XFS_FSB_TO_DADDR(mp, bno)) {
			do_warn(
_("expected block %" PRIu64 ", got %llu, bmbt block %" PRIu64 "\n"),
				XFS_FSB_TO_DADDR(mp, bno),
				(unsigned long long)be64_to_cpu(block->bb_u.l.bb_blkno),
				bno);
			return 1;
		}
		/* verify uuid */
		if (platform_uuid_compare(&block->bb_u.l.bb_uuid,
					  &mp->m_sb.sb_meta_uuid) != 0) {
			do_warn(
_("wrong FS UUID, bmbt block %" PRIu64 "\n"),
				bno);
			return 1;
		}
	}

	if (check_dups == 0)  {
		/*
		 * check sibling pointers. if bad we have a conflict
		 * between the sibling pointers and the child pointers
		 * in the parent block.  blow out the inode if that happens
		 */
		if (bm_cursor->level[level].fsbno != NULLFSBLOCK)  {
			/*
			 * this is not the first block on this level
			 * so the cursor for this level has recorded the
			 * values for this's block left-sibling.
			 */
			if (bno != bm_cursor->level[level].right_fsbno)  {
				do_warn(
_("bad fwd (right) sibling pointer (saw %" PRIu64 " parent block says %" PRIu64 ")\n"
  "\tin inode %" PRIu64 " (%s fork) bmap btree block %" PRIu64 "\n"),
					bm_cursor->level[level].right_fsbno,
					bno, ino, forkname,
					bm_cursor->level[level].fsbno);
				return(1);
			}
			if (be64_to_cpu(block->bb_u.l.bb_leftsib) !=
					bm_cursor->level[level].fsbno)  {
				do_warn(
_("bad back (left) sibling pointer (saw %llu parent block says %" PRIu64 ")\n"
  "\tin inode %" PRIu64 " (%s fork) bmap btree block %" PRIu64 "\n"),
				       (unsigned long long)
					       be64_to_cpu(block->bb_u.l.bb_leftsib),
					bm_cursor->level[level].fsbno,
					ino, forkname, bno);
				return(1);
			}
		} else {
			/*
			 * This is the first or only block on this level.
			 * Check that the left sibling pointer is NULL
			 */
			if (be64_to_cpu(block->bb_u.l.bb_leftsib) != NULLFSBLOCK) {
				do_warn(
_("bad back (left) sibling pointer (saw %llu should be NULL (0))\n"
  "\tin inode %" PRIu64 " (%s fork) bmap btree block %" PRIu64 "\n"),
				       (unsigned long long)
					       be64_to_cpu(block->bb_u.l.bb_leftsib),
					ino, forkname, bno);
				return(1);
			}
		}

		/*
		 * update cursor block pointers to reflect this block
		 */
		bm_cursor->level[level].fsbno = bno;
		bm_cursor->level[level].left_fsbno =
					be64_to_cpu(block->bb_u.l.bb_leftsib);
		bm_cursor->level[level].right_fsbno =
					be64_to_cpu(block->bb_u.l.bb_rightsib);

		agno = XFS_FSB_TO_AGNO(mp, bno);
		agbno = XFS_FSB_TO_AGBNO(mp, bno);

		pthread_mutex_lock(&ag_locks[agno].lock);
		state = get_bmap(agno, agbno);
		switch (state) {
		case XR_E_INUSE1:
			/*
			 * block was claimed as in use data by the rmap
			 * btree, but has not been found in the data extent
			 * map for the inode. That means this bmbt block hasn't
			 * yet been claimed as in use, which means -it's ours-
			 */
		case XR_E_UNKNOWN:
		case XR_E_FREE1:
		case XR_E_FREE:
			set_bmap(agno, agbno, XR_E_INUSE);
			break;
		case XR_E_FS_MAP:
		case XR_E_INUSE:
			/*
			 * we'll try and continue searching here since
			 * the block looks like it's been claimed by file
			 * to store user data, a directory to store directory
			 * data, or the space allocation btrees but since
			 * we made it here, the block probably
			 * contains btree data.
			 */
			set_bmap(agno, agbno, XR_E_MULT);
			do_warn(
_("inode 0x%" PRIx64 "bmap block 0x%" PRIx64 " claimed, state is %d\n"),
				ino, bno, state);
			break;
		case XR_E_MULT:
		case XR_E_INUSE_FS:
			set_bmap(agno, agbno, XR_E_MULT);
			do_warn(
_("inode 0x%" PRIx64 " bmap block 0x%" PRIx64 " claimed, state is %d\n"),
				ino, bno, state);
			/*
			 * if we made it to here, this is probably a bmap block
			 * that is being used by *another* file as a bmap block
			 * so the block will be valid.  Both files should be
			 * trashed along with any other file that impinges on
			 * any blocks referenced by either file.  So we
			 * continue searching down this btree to mark all
			 * blocks duplicate
			 */
			break;
		case XR_E_BAD_STATE:
		default:
			do_warn(
_("bad state %d, inode %" PRIu64 " bmap block 0x%" PRIx64 "\n"),
				state, ino, bno);
			break;
		}
		pthread_mutex_unlock(&ag_locks[agno].lock);
	} else  {
		/*
		 * attribute fork for realtime files is in the regular
		 * filesystem
		 */
		if (type != XR_INO_RTDATA || whichfork != XFS_DATA_FORK)  {
			if (search_dup_extent(XFS_FSB_TO_AGNO(mp, bno),
					XFS_FSB_TO_AGBNO(mp, bno),
					XFS_FSB_TO_AGBNO(mp, bno) + 1))
				return(1);
		} else  {
			if (search_rt_dup_extent(mp, bno))
				return(1);
		}
	}
	(*tot)++;
	numrecs = be16_to_cpu(block->bb_numrecs);

	/* Record BMBT blocks in the reverse-mapping data. */
	if (check_dups && collect_rmaps) {
		agno = XFS_FSB_TO_AGNO(mp, bno);
		pthread_mutex_lock(&ag_locks[agno].lock);
		error = rmap_add_bmbt_rec(mp, ino, whichfork, bno);
		pthread_mutex_unlock(&ag_locks[agno].lock);
		if (error)
			do_error(
_("couldn't add inode %"PRIu64" bmbt block %"PRIu64" reverse-mapping data."),
				ino, bno);
	}

	if (level == 0) {
		if (numrecs > mp->m_bmap_dmxr[0] || (isroot == 0 && numrecs <
							mp->m_bmap_dmnr[0])) {
				do_warn(
_("inode %" PRIu64 " bad # of bmap records (%u, min - %u, max - %u)\n"),
					ino, numrecs, mp->m_bmap_dmnr[0],
					mp->m_bmap_dmxr[0]);
			return(1);
		}
		rp = XFS_BMBT_REC_ADDR(mp, block, 1);
		*nex += numrecs;
		/*
		 * XXX - if we were going to fix up the btree record,
		 * we'd do it right here.  For now, if there's a problem,
		 * we'll bail out and presumably clear the inode.
		 */
		if (check_dups == 0)  {
			err = process_bmbt_reclist(mp, rp, &numrecs, type, ino,
						   tot, blkmapp, &first_key,
						   &last_key, whichfork);
			if (err)
				return 1;

			/*
			 * check that key ordering is monotonically increasing.
			 * if the last_key value in the cursor is set to
			 * NULLFILEOFF, then we know this is the first block
			 * on the leaf level and we shouldn't check the
			 * last_key value.
			 */
			if (first_key <= bm_cursor->level[level].last_key &&
					bm_cursor->level[level].last_key !=
					NULLFILEOFF)  {
				do_warn(
_("out-of-order bmap key (file offset) in inode %" PRIu64 ", %s fork, fsbno %" PRIu64 "\n"),
					ino, forkname, bno);
				return(1);
			}
			/*
			 * update cursor keys to reflect this block.
			 * don't have to check if last_key is > first_key
			 * since that gets checked by process_bmbt_reclist.
			 */
			bm_cursor->level[level].first_key = first_key;
			bm_cursor->level[level].last_key = last_key;

			return 0;
		} else {
			return scan_bmbt_reclist(mp, rp, &numrecs, type, ino,
						 tot, whichfork);
		}
	}
	if (numrecs > mp->m_bmap_dmxr[1] || (isroot == 0 && numrecs <
							mp->m_bmap_dmnr[1])) {
		do_warn(
_("inode %" PRIu64 " bad # of bmap records (%u, min - %u, max - %u)\n"),
			ino, numrecs, mp->m_bmap_dmnr[1], mp->m_bmap_dmxr[1]);
		return(1);
	}
	pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[1]);
	pkey = XFS_BMBT_KEY_ADDR(mp, block, 1);

	last_key = NULLFILEOFF;

	for (i = 0, err = 0; i < numrecs; i++)  {
		/*
		 * XXX - if we were going to fix up the interior btree nodes,
		 * we'd do it right here.  For now, if there's a problem,
		 * we'll bail out and presumably clear the inode.
		 */
		if (!verify_dfsbno(mp, be64_to_cpu(pp[i])))  {
			do_warn(
_("bad bmap btree ptr 0x%llx in ino %" PRIu64 "\n"),
			       (unsigned long long) be64_to_cpu(pp[i]), ino);
			return(1);
		}

		err = scan_lbtree(be64_to_cpu(pp[i]), level, scan_bmapbt,
				type, whichfork, ino, tot, nex, blkmapp,
				bm_cursor, 0, check_dups, magic,
				&xfs_bmbt_buf_ops);
		if (err)
			return(1);

		/*
		 * fix key (offset) mismatches between the first key
		 * in the child block (as recorded in the cursor) and the
		 * key in the interior node referencing the child block.
		 *
		 * fixes cases where entries have been shifted between
		 * child blocks but the parent hasn't been updated.  We
		 * don't have to worry about the key values in the cursor
		 * not being set since we only look at the key values of
		 * our child and those are guaranteed to be set by the
		 * call to scan_lbtree() above.
		 */
		if (check_dups == 0 && be64_to_cpu(pkey[i].br_startoff) !=
					bm_cursor->level[level-1].first_key)  {
			if (!no_modify)  {
				do_warn(
_("correcting bt key (was %llu, now %" PRIu64 ") in inode %" PRIu64 "\n"
  "\t\t%s fork, btree block %" PRIu64 "\n"),
				       (unsigned long long)
					       be64_to_cpu(pkey[i].br_startoff),
					bm_cursor->level[level-1].first_key,
					ino,
					forkname, bno);
				*dirty = 1;
				pkey[i].br_startoff = cpu_to_be64(
					bm_cursor->level[level-1].first_key);
			} else  {
				do_warn(
_("bad btree key (is %llu, should be %" PRIu64 ") in inode %" PRIu64 "\n"
  "\t\t%s fork, btree block %" PRIu64 "\n"),
				       (unsigned long long)
					       be64_to_cpu(pkey[i].br_startoff),
					bm_cursor->level[level-1].first_key,
					ino, forkname, bno);
			}
		}
	}

	/*
	 * If we're the last node at our level, check that the last child
	 * block's forward sibling pointer is NULL.
	 */
	if (check_dups == 0 &&
			bm_cursor->level[level].right_fsbno == NULLFSBLOCK &&
			bm_cursor->level[level - 1].right_fsbno != NULLFSBLOCK) {
		do_warn(
_("bad fwd (right) sibling pointer (saw %" PRIu64 " should be NULLFSBLOCK)\n"
  "\tin inode %" PRIu64 " (%s fork) bmap btree block %" PRIu64 "\n"),
			bm_cursor->level[level - 1].right_fsbno,
			ino, forkname, bm_cursor->level[level - 1].fsbno);
		return(1);
	}

	/*
	 * update cursor keys to reflect this block
	 */
	if (check_dups == 0)  {
		bm_cursor->level[level].first_key =
				be64_to_cpu(pkey[0].br_startoff);
		bm_cursor->level[level].last_key =
				be64_to_cpu(pkey[numrecs - 1].br_startoff);
	}

	return(0);
}

static void
scan_allocbt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	uint32_t		magic,
	void			*priv)
{
	struct aghdr_cnts	*agcnts = priv;
	const char 		*name;
	int			i;
	xfs_alloc_ptr_t		*pp;
	xfs_alloc_rec_t		*rp;
	int			hdr_errors = 0;
	int			numrecs;
	int			state;
	xfs_extlen_t		lastcount = 0;
	xfs_agblock_t		lastblock = 0;

	switch (magic) {
	case XFS_ABTB_CRC_MAGIC:
	case XFS_ABTB_MAGIC:
		name = "bno";
		break;
	case XFS_ABTC_CRC_MAGIC:
	case XFS_ABTC_MAGIC:
		name = "cnt";
		break;
	default:
		name = "(unknown)";
		assert(0);
		break;
	}

	if (be32_to_cpu(block->bb_magic) != magic) {
		do_warn(_("bad magic # %#x in bt%s block %d/%d\n"),
			be32_to_cpu(block->bb_magic), name, agno, bno);
		hdr_errors++;
		if (suspect)
			return;
	}

	/*
	 * All freespace btree blocks except the roots are freed for a
	 * fully used filesystem, thus they are counted towards the
	 * free data block counter.
	 */
	if (!isroot) {
		agcnts->agfbtreeblks++;
		agcnts->fdblocks++;
	}

	if (be16_to_cpu(block->bb_level) != level) {
		do_warn(_("expected level %d got %d in bt%s block %d/%d\n"),
			level, be16_to_cpu(block->bb_level), name, agno, bno);
		hdr_errors++;
		if (suspect)
			return;
	}

	/*
	 * check for btree blocks multiply claimed
	 */
	state = get_bmap(agno, bno);
	if (state != XR_E_UNKNOWN)  {
		set_bmap(agno, bno, XR_E_MULT);
		do_warn(
_("%s freespace btree block claimed (state %d), agno %d, bno %d, suspect %d\n"),
				name, state, agno, bno, suspect);
		return;
	}
	set_bmap(agno, bno, XR_E_FS_MAP);

	numrecs = be16_to_cpu(block->bb_numrecs);

	if (level == 0) {
		if (numrecs > mp->m_alloc_mxr[0])  {
			numrecs = mp->m_alloc_mxr[0];
			hdr_errors++;
		}
		if (isroot == 0 && numrecs < mp->m_alloc_mnr[0])  {
			numrecs = mp->m_alloc_mnr[0];
			hdr_errors++;
		}

		if (hdr_errors) {
			do_warn(
	_("bad btree nrecs (%u, min=%u, max=%u) in bt%s block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs),
				mp->m_alloc_mnr[0], mp->m_alloc_mxr[0],
				name, agno, bno);
			suspect++;
		}

		rp = XFS_ALLOC_REC_ADDR(mp, block, 1);
		for (i = 0; i < numrecs; i++) {
			xfs_agblock_t		b, end;
			xfs_extlen_t		len, blen;

			b = be32_to_cpu(rp[i].ar_startblock);
			len = be32_to_cpu(rp[i].ar_blockcount);
			end = b + len;

			if (b == 0 || !verify_agbno(mp, agno, b)) {
				do_warn(
	_("invalid start block %u in record %u of %s btree block %u/%u\n"),
					b, i, name, agno, bno);
				continue;
			}
			if (len == 0 || !verify_agbno(mp, agno, end - 1)) {
				do_warn(
	_("invalid length %u in record %u of %s btree block %u/%u\n"),
					len, i, name, agno, bno);
				continue;
			}

			if (magic == XFS_ABTB_MAGIC ||
			    magic == XFS_ABTB_CRC_MAGIC) {
				if (b <= lastblock) {
					do_warn(_(
	"out-of-order bno btree record %d (%u %u) block %u/%u\n"),
						i, b, len, agno, bno);
				} else {
					lastblock = b;
				}
			} else {
				agcnts->fdblocks += len;
				agcnts->agffreeblks += len;
				if (len > agcnts->agflongest)
					agcnts->agflongest = len;
				if (len < lastcount) {
					do_warn(_(
	"out-of-order cnt btree record %d (%u %u) block %u/%u\n"),
						i, b, len, agno, bno);
				} else {
					lastcount = len;
				}
			}

			for ( ; b < end; b += blen)  {
				state = get_bmap_ext(agno, b, end, &blen);
				switch (state) {
				case XR_E_UNKNOWN:
					set_bmap(agno, b, XR_E_FREE1);
					break;
				case XR_E_FREE1:
					/*
					 * no warning messages -- we'll catch
					 * FREE1 blocks later
					 */
					if (magic == XFS_ABTC_MAGIC ||
					    magic == XFS_ABTC_CRC_MAGIC) {
						set_bmap_ext(agno, b, blen,
							     XR_E_FREE);
						break;
					}
				default:
					do_warn(
	_("block (%d,%d-%d) multiply claimed by %s space tree, state - %d\n"),
						agno, b, b + blen - 1,
						name, state);
					break;
				}
			}
		}
		return;
	}

	/*
	 * interior record
	 */
	pp = XFS_ALLOC_PTR_ADDR(mp, block, 1, mp->m_alloc_mxr[1]);

	if (numrecs > mp->m_alloc_mxr[1])  {
		numrecs = mp->m_alloc_mxr[1];
		hdr_errors++;
	}
	if (isroot == 0 && numrecs < mp->m_alloc_mnr[1])  {
		numrecs = mp->m_alloc_mnr[1];
		hdr_errors++;
	}

	/*
	 * don't pass bogus tree flag down further if this block
	 * looked ok.  bail out if two levels in a row look bad.
	 */
	if (hdr_errors)  {
		do_warn(
	_("bad btree nrecs (%u, min=%u, max=%u) in bt%s block %u/%u\n"),
			be16_to_cpu(block->bb_numrecs),
			mp->m_alloc_mnr[1], mp->m_alloc_mxr[1],
			name, agno, bno);
		if (suspect)
			return;
		suspect++;
	} else if (suspect) {
		suspect = 0;
	}

	for (i = 0; i < numrecs; i++)  {
		xfs_agblock_t		bno = be32_to_cpu(pp[i]);

		/*
		 * XXX - put sibling detection right here.
		 * we know our sibling chain is good.  So as we go,
		 * we check the entry before and after each entry.
		 * If either of the entries references a different block,
		 * check the sibling pointer.  If there's a sibling
		 * pointer mismatch, try and extract as much data
		 * as possible.
		 */
		if (bno != 0 && verify_agbno(mp, agno, bno)) {
			switch (magic) {
			case XFS_ABTB_CRC_MAGIC:
			case XFS_ABTB_MAGIC:
				scan_sbtree(bno, level, agno, suspect,
					    scan_allocbt, 0, magic, priv,
					    &xfs_allocbt_buf_ops);
				break;
			case XFS_ABTC_CRC_MAGIC:
			case XFS_ABTC_MAGIC:
				scan_sbtree(bno, level, agno, suspect,
					    scan_allocbt, 0, magic, priv,
					    &xfs_allocbt_buf_ops);
				break;
			}
		}
	}
}

static bool
ino_issparse(
	struct xfs_inobt_rec	*rp,
	int			offset)
{
	if (!xfs_sb_version_hassparseinodes(&mp->m_sb))
		return false;

	return xfs_inobt_is_sparse_disk(rp, offset);
}

/* See if the rmapbt owners agree with our observations. */
static void
process_rmap_rec(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agblock_t		b,
	xfs_agblock_t		end,
	xfs_extlen_t		blen,
	int64_t			owner,
	int			state,
	const char		*name)
{
	switch (state) {
	case XR_E_UNKNOWN:
		switch (owner) {
		case XFS_RMAP_OWN_FS:
		case XFS_RMAP_OWN_LOG:
			set_bmap_ext(agno, b, blen, XR_E_INUSE_FS1);
			break;
		case XFS_RMAP_OWN_AG:
		case XFS_RMAP_OWN_INOBT:
			set_bmap_ext(agno, b, blen, XR_E_FS_MAP1);
			break;
		case XFS_RMAP_OWN_INODES:
			set_bmap_ext(agno, b, blen, XR_E_INO1);
			break;
		case XFS_RMAP_OWN_REFC:
			set_bmap_ext(agno, b, blen, XR_E_REFC);
			break;
		case XFS_RMAP_OWN_COW:
			set_bmap_ext(agno, b, blen, XR_E_COW);
			break;
		case XFS_RMAP_OWN_NULL:
			/* still unknown */
			break;
		default:
			/* file data */
			set_bmap_ext(agno, b, blen, XR_E_INUSE1);
			break;
		}
		break;
	case XR_E_INUSE_FS:
		if (owner == XFS_RMAP_OWN_FS ||
		    owner == XFS_RMAP_OWN_LOG)
			break;
		do_warn(
_("Static meta block (%d,%d-%d) mismatch in %s tree, state - %d,%" PRIx64 "\n"),
			agno, b, b + blen - 1,
			name, state, owner);
		break;
	case XR_E_FS_MAP:
		if (owner == XFS_RMAP_OWN_AG ||
		    owner == XFS_RMAP_OWN_INOBT)
			break;
		do_warn(
_("AG meta block (%d,%d-%d) mismatch in %s tree, state - %d,%" PRIx64 "\n"),
			agno, b, b + blen - 1,
			name, state, owner);
		break;
	case XR_E_INO:
		if (owner == XFS_RMAP_OWN_INODES)
			break;
		do_warn(
_("inode block (%d,%d-%d) mismatch in %s tree, state - %d,%" PRIx64 "\n"),
			agno, b, b + blen - 1,
			name, state, owner);
		break;
	case XR_E_REFC:
		if (owner == XFS_RMAP_OWN_REFC)
			break;
		do_warn(
_("AG refcount block (%d,%d-%d) mismatch in %s tree, state - %d,%" PRIx64 "\n"),
			agno, b, b + blen - 1,
			name, state, owner);
		break;
	case XR_E_INUSE:
		if (owner >= 0 &&
		    owner < mp->m_sb.sb_dblocks)
			break;
		do_warn(
_("in use block (%d,%d-%d) mismatch in %s tree, state - %d,%" PRIx64 "\n"),
			agno, b, b + blen - 1,
			name, state, owner);
		break;
	case XR_E_FREE1:
	case XR_E_FREE:
		/*
		 * May be on the AGFL. If not, they'll
		 * be caught later.
		 */
		break;
	case XR_E_INUSE1:
		/*
		 * multiple inode owners are ok with
		 * reflink enabled
		 */
		if (xfs_sb_version_hasreflink(&mp->m_sb) &&
		    !XFS_RMAP_NON_INODE_OWNER(owner))
			break;
		/* fall through */
	default:
		do_warn(
_("unknown block (%d,%d-%d) mismatch on %s tree, state - %d,%" PRIx64 "\n"),
			agno, b, b + blen - 1,
			name, state, owner);
		break;
	}
}

struct rmap_priv {
	struct aghdr_cnts	*agcnts;
	struct xfs_rmap_irec	high_key;
	struct xfs_rmap_irec	last_rec;
	xfs_agblock_t		nr_blocks;
};

static bool
rmap_in_order(
	xfs_agblock_t	b,
	xfs_agblock_t	lastblock,
	uint64_t	owner,
	uint64_t	lastowner,
	uint64_t	offset,
	uint64_t	lastoffset)
{
	if (b > lastblock)
		return true;
	else if (b < lastblock)
		return false;

	if (owner > lastowner)
		return true;
	else if (owner < lastowner)
		return false;

	return offset > lastoffset;
}

static void
scan_rmapbt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	uint32_t		magic,
	void			*priv)
{
	const char		*name = "rmap";
	int			i;
	xfs_rmap_ptr_t		*pp;
	struct xfs_rmap_rec	*rp;
	struct rmap_priv	*rmap_priv = priv;
	int			hdr_errors = 0;
	int			numrecs;
	int			state;
	xfs_agblock_t		lastblock = 0;
	uint64_t		lastowner = 0;
	uint64_t		lastoffset = 0;
	struct xfs_rmap_key	*kp;
	struct xfs_rmap_irec	key = {0};

	if (magic != XFS_RMAP_CRC_MAGIC) {
		name = "(unknown)";
		hdr_errors++;
		suspect++;
		goto out;
	}

	if (be32_to_cpu(block->bb_magic) != magic) {
		do_warn(_("bad magic # %#x in bt%s block %d/%d\n"),
			be32_to_cpu(block->bb_magic), name, agno, bno);
		hdr_errors++;
		if (suspect)
			goto out;
	}

	/*
	 * All RMAP btree blocks except the roots are freed for a
	 * fully empty filesystem, thus they are counted towards the
	 * free data block counter.
	 */
	if (!isroot) {
		rmap_priv->agcnts->agfbtreeblks++;
		rmap_priv->agcnts->fdblocks++;
	}
	rmap_priv->nr_blocks++;

	if (be16_to_cpu(block->bb_level) != level) {
		do_warn(_("expected level %d got %d in bt%s block %d/%d\n"),
			level, be16_to_cpu(block->bb_level), name, agno, bno);
		hdr_errors++;
		if (suspect)
			goto out;
	}

	/* check for btree blocks multiply claimed */
	state = get_bmap(agno, bno);
	if (!(state == XR_E_UNKNOWN || state == XR_E_FS_MAP1))  {
		set_bmap(agno, bno, XR_E_MULT);
		do_warn(
_("%s rmap btree block claimed (state %d), agno %d, bno %d, suspect %d\n"),
				name, state, agno, bno, suspect);
		goto out;
	}
	set_bmap(agno, bno, XR_E_FS_MAP);

	numrecs = be16_to_cpu(block->bb_numrecs);
	if (level == 0) {
		if (numrecs > mp->m_rmap_mxr[0])  {
			numrecs = mp->m_rmap_mxr[0];
			hdr_errors++;
		}
		if (isroot == 0 && numrecs < mp->m_rmap_mnr[0])  {
			numrecs = mp->m_rmap_mnr[0];
			hdr_errors++;
		}

		if (hdr_errors) {
			do_warn(
	_("bad btree nrecs (%u, min=%u, max=%u) in bt%s block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs),
				mp->m_rmap_mnr[0], mp->m_rmap_mxr[0],
				name, agno, bno);
			suspect++;
		}

		rp = XFS_RMAP_REC_ADDR(block, 1);
		for (i = 0; i < numrecs; i++) {
			xfs_agblock_t		b, end;
			xfs_extlen_t		len, blen;
			int64_t			owner, offset;

			b = be32_to_cpu(rp[i].rm_startblock);
			len = be32_to_cpu(rp[i].rm_blockcount);
			owner = be64_to_cpu(rp[i].rm_owner);
			offset = be64_to_cpu(rp[i].rm_offset);

			key.rm_flags = 0;
			key.rm_startblock = b;
			key.rm_blockcount = len;
			key.rm_owner = owner;
			if (libxfs_rmap_irec_offset_unpack(offset, &key)) {
				/* Look for impossible flags. */
				do_warn(
	_("invalid flags in record %u of %s btree block %u/%u\n"),
					i, name, agno, bno);
				continue;
			}

			end = key.rm_startblock + key.rm_blockcount;

			/* Make sure agbno & len make sense. */
			if (!verify_agbno(mp, agno, b)) {
				do_warn(
	_("invalid start block %u in record %u of %s btree block %u/%u\n"),
					b, i, name, agno, bno);
				continue;
			}
			if (len == 0 || !verify_agbno(mp, agno, end - 1)) {
				do_warn(
	_("invalid length %u in record %u of %s btree block %u/%u\n"),
					len, i, name, agno, bno);
				continue;
			}

			/* Look for impossible owners. */
			if (!((owner > XFS_RMAP_OWN_MIN &&
			       owner <= XFS_RMAP_OWN_FS) ||
			      (XFS_INO_TO_AGNO(mp, owner) < mp->m_sb.sb_agcount &&
			       XFS_AGINO_TO_AGBNO(mp,
					XFS_INO_TO_AGINO(mp, owner)) <
					mp->m_sb.sb_agblocks)))
				do_warn(
	_("invalid owner in rmap btree record %d (%"PRId64" %u) block %u/%u\n"),
						i, owner, len, agno, bno);

			/* Look for impossible record field combinations. */
			if (XFS_RMAP_NON_INODE_OWNER(key.rm_owner)) {
				if (key.rm_flags)
					do_warn(
	_("record %d of block (%u/%u) in %s btree cannot have non-inode owner with flags\n"),
						i, agno, bno, name);
				if (key.rm_offset)
					do_warn(
	_("record %d of block (%u/%u) in %s btree cannot have non-inode owner with offset\n"),
						i, agno, bno, name);
			}

			/* Check for out of order records. */
			if (i == 0) {
advance:
				lastblock = b;
				lastowner = owner;
				lastoffset = offset;
			} else {
				bool bad;

				if (xfs_sb_version_hasreflink(&mp->m_sb))
					bad = !rmap_in_order(b, lastblock,
							owner, lastowner,
							offset, lastoffset);
				else
					bad = b <= lastblock;
				if (bad)
					do_warn(
	_("out-of-order rmap btree record %d (%u %"PRId64" %"PRIx64" %u) block %u/%u\n"),
					i, b, owner, offset, len, agno, bno);
				else
					goto advance;
			}

			/* Is this mergeable with the previous record? */
			if (rmaps_are_mergeable(&rmap_priv->last_rec, &key)) {
				do_warn(
	_("record %d in block (%u/%u) of %s tree should be merged with previous record\n"),
					i, agno, bno, name);
				rmap_priv->last_rec.rm_blockcount +=
						key.rm_blockcount;
			} else
				rmap_priv->last_rec = key;

			/* Check that we don't go past the high key. */
			key.rm_startblock += key.rm_blockcount - 1;
			if (!XFS_RMAP_NON_INODE_OWNER(key.rm_owner) &&
			    !(key.rm_flags & XFS_RMAP_BMBT_BLOCK))
				key.rm_offset += key.rm_blockcount - 1;
			key.rm_blockcount = 0;
			if (rmap_diffkeys(&key, &rmap_priv->high_key) > 0) {
				do_warn(
	_("record %d greater than high key of block (%u/%u) in %s tree\n"),
					i, agno, bno, name);
			}

			/* Check for block owner collisions. */
			for ( ; b < end; b += blen)  {
				state = get_bmap_ext(agno, b, end, &blen);
				process_rmap_rec(mp, agno, b, end, blen, owner,
						state, name);
			}
		}
		goto out;
	}

	/*
	 * interior record
	 */
	pp = XFS_RMAP_PTR_ADDR(block, 1, mp->m_rmap_mxr[1]);

	if (numrecs > mp->m_rmap_mxr[1])  {
		numrecs = mp->m_rmap_mxr[1];
		hdr_errors++;
	}
	if (isroot == 0 && numrecs < mp->m_rmap_mnr[1])  {
		numrecs = mp->m_rmap_mnr[1];
		hdr_errors++;
	}

	/*
	 * don't pass bogus tree flag down further if this block
	 * looked ok.  bail out if two levels in a row look bad.
	 */
	if (hdr_errors)  {
		do_warn(
	_("bad btree nrecs (%u, min=%u, max=%u) in bt%s block %u/%u\n"),
			be16_to_cpu(block->bb_numrecs),
			mp->m_rmap_mnr[1], mp->m_rmap_mxr[1],
			name, agno, bno);
		if (suspect)
			goto out;
		suspect++;
	} else if (suspect) {
		suspect = 0;
	}

	/* check the node's high keys */
	for (i = 0; !isroot && i < numrecs; i++) {
		kp = XFS_RMAP_HIGH_KEY_ADDR(block, i + 1);

		key.rm_flags = 0;
		key.rm_startblock = be32_to_cpu(kp->rm_startblock);
		key.rm_owner = be64_to_cpu(kp->rm_owner);
		if (libxfs_rmap_irec_offset_unpack(be64_to_cpu(kp->rm_offset),
				&key)) {
			/* Look for impossible flags. */
			do_warn(
	_("invalid flags in key %u of %s btree block %u/%u\n"),
				i, name, agno, bno);
			continue;
		}
		if (rmap_diffkeys(&key, &rmap_priv->high_key) > 0)
			do_warn(
	_("key %d greater than high key of block (%u/%u) in %s tree\n"),
				i, agno, bno, name);
	}

	for (i = 0; i < numrecs; i++)  {
		xfs_agblock_t		bno = be32_to_cpu(pp[i]);

		/*
		 * XXX - put sibling detection right here.
		 * we know our sibling chain is good.  So as we go,
		 * we check the entry before and after each entry.
		 * If either of the entries references a different block,
		 * check the sibling pointer.  If there's a sibling
		 * pointer mismatch, try and extract as much data
		 * as possible.
		 */
		kp = XFS_RMAP_HIGH_KEY_ADDR(block, i + 1);
		rmap_priv->high_key.rm_flags = 0;
		rmap_priv->high_key.rm_startblock =
				be32_to_cpu(kp->rm_startblock);
		rmap_priv->high_key.rm_owner =
				be64_to_cpu(kp->rm_owner);
		if (libxfs_rmap_irec_offset_unpack(be64_to_cpu(kp->rm_offset),
				&rmap_priv->high_key)) {
			/* Look for impossible flags. */
			do_warn(
	_("invalid flags in high key %u of %s btree block %u/%u\n"),
				i, name, agno, bno);
			continue;
		}

		if (bno != 0 && verify_agbno(mp, agno, bno)) {
			scan_sbtree(bno, level, agno, suspect, scan_rmapbt, 0,
				    magic, priv, &xfs_rmapbt_buf_ops);
		}
	}

out:
	if (suspect)
		rmap_avoid_check();
}

struct refc_priv {
	struct xfs_refcount_irec	last_rec;
	xfs_agblock_t			nr_blocks;
};


static void
scan_refcbt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	uint32_t		magic,
	void			*priv)
{
	const char		*name = "refcount";
	int			i;
	xfs_refcount_ptr_t	*pp;
	struct xfs_refcount_rec	*rp;
	int			hdr_errors = 0;
	int			numrecs;
	int			state;
	xfs_agblock_t		lastblock = 0;
	struct refc_priv	*refc_priv = priv;

	if (magic != XFS_REFC_CRC_MAGIC) {
		name = "(unknown)";
		hdr_errors++;
		suspect++;
		goto out;
	}

	if (be32_to_cpu(block->bb_magic) != magic) {
		do_warn(_("bad magic # %#x in %s btree block %d/%d\n"),
			be32_to_cpu(block->bb_magic), name, agno, bno);
		hdr_errors++;
		if (suspect)
			goto out;
	}

	if (be16_to_cpu(block->bb_level) != level) {
		do_warn(_("expected level %d got %d in %s btree block %d/%d\n"),
			level, be16_to_cpu(block->bb_level), name, agno, bno);
		hdr_errors++;
		if (suspect)
			goto out;
	}

	refc_priv->nr_blocks++;

	/* check for btree blocks multiply claimed */
	state = get_bmap(agno, bno);
	if (!(state == XR_E_UNKNOWN || state == XR_E_REFC))  {
		set_bmap(agno, bno, XR_E_MULT);
		do_warn(
_("%s btree block claimed (state %d), agno %d, bno %d, suspect %d\n"),
				name, state, agno, bno, suspect);
		goto out;
	}
	set_bmap(agno, bno, XR_E_FS_MAP);

	numrecs = be16_to_cpu(block->bb_numrecs);
	if (level == 0) {
		if (numrecs > mp->m_refc_mxr[0])  {
			numrecs = mp->m_refc_mxr[0];
			hdr_errors++;
		}
		if (isroot == 0 && numrecs < mp->m_refc_mnr[0])  {
			numrecs = mp->m_refc_mnr[0];
			hdr_errors++;
		}

		if (hdr_errors) {
			do_warn(
	_("bad btree nrecs (%u, min=%u, max=%u) in %s btree block %u/%u\n"),
				be16_to_cpu(block->bb_numrecs),
				mp->m_refc_mnr[0], mp->m_refc_mxr[0],
				name, agno, bno);
			suspect++;
		}

		rp = XFS_REFCOUNT_REC_ADDR(block, 1);
		for (i = 0; i < numrecs; i++) {
			xfs_agblock_t		b, agb, end;
			xfs_extlen_t		len;
			xfs_nlink_t		nr;

			b = agb = be32_to_cpu(rp[i].rc_startblock);
			len = be32_to_cpu(rp[i].rc_blockcount);
			nr = be32_to_cpu(rp[i].rc_refcount);
			if (b >= XFS_REFC_COW_START && nr != 1)
				do_warn(
_("leftover CoW extent has incorrect refcount in record %u of %s btree block %u/%u\n"),
					i, name, agno, bno);
			if (nr == 1) {
				if (agb < XFS_REFC_COW_START)
					do_warn(
_("leftover CoW extent has invalid startblock in record %u of %s btree block %u/%u\n"),
						i, name, agno, bno);
				agb -= XFS_REFC_COW_START;
			}
			end = agb + len;

			if (!verify_agbno(mp, agno, agb)) {
				do_warn(
	_("invalid start block %u in record %u of %s btree block %u/%u\n"),
					b, i, name, agno, bno);
				continue;
			}
			if (len == 0 || !verify_agbno(mp, agno, end - 1)) {
				do_warn(
	_("invalid length %u in record %u of %s btree block %u/%u\n"),
					len, i, name, agno, bno);
				continue;
			}

			if (nr == 1) {
				xfs_agblock_t	c;
				xfs_extlen_t	cnr;

				for (c = agb; c < end; c += cnr) {
					state = get_bmap_ext(agno, c, end, &cnr);
					switch (state) {
					case XR_E_UNKNOWN:
					case XR_E_COW:
						do_warn(
_("leftover CoW extent (%u/%u) len %u\n"),
						agno, c, cnr);
						set_bmap_ext(agno, c, cnr, XR_E_FREE);
						break;
					default:
						do_warn(
_("extent (%u/%u) len %u claimed, state is %d\n"),
						agno, c, cnr, state);
						break;
					}
				}
			} else if (nr < 2 || nr > MAXREFCOUNT) {
				do_warn(
	_("invalid reference count %u in record %u of %s btree block %u/%u\n"),
					nr, i, name, agno, bno);
				continue;
			}

			if (b && b <= lastblock) {
				do_warn(_(
	"out-of-order %s btree record %d (%u %u) block %u/%u\n"),
					name, i, b, len, agno, bno);
			} else {
				lastblock = b;
			}

			/* Is this record mergeable with the last one? */
			if (refc_priv->last_rec.rc_startblock +
			    refc_priv->last_rec.rc_blockcount == b &&
			    refc_priv->last_rec.rc_refcount == nr) {
				do_warn(
	_("record %d in block (%u/%u) of %s tree should be merged with previous record\n"),
					i, agno, bno, name);
				refc_priv->last_rec.rc_blockcount += len;
			} else {
				refc_priv->last_rec.rc_startblock = b;
				refc_priv->last_rec.rc_blockcount = len;
				refc_priv->last_rec.rc_refcount = nr;
			}

			/* XXX: probably want to mark the reflinked areas? */
		}
		goto out;
	}

	/*
	 * interior record
	 */
	pp = XFS_REFCOUNT_PTR_ADDR(block, 1, mp->m_refc_mxr[1]);

	if (numrecs > mp->m_refc_mxr[1])  {
		numrecs = mp->m_refc_mxr[1];
		hdr_errors++;
	}
	if (isroot == 0 && numrecs < mp->m_refc_mnr[1])  {
		numrecs = mp->m_refc_mnr[1];
		hdr_errors++;
	}

	/*
	 * don't pass bogus tree flag down further if this block
	 * looked ok.  bail out if two levels in a row look bad.
	 */
	if (hdr_errors)  {
		do_warn(
	_("bad btree nrecs (%u, min=%u, max=%u) in %s btree block %u/%u\n"),
			be16_to_cpu(block->bb_numrecs),
			mp->m_refc_mnr[1], mp->m_refc_mxr[1],
			name, agno, bno);
		if (suspect)
			goto out;
		suspect++;
	} else if (suspect) {
		suspect = 0;
	}

	for (i = 0; i < numrecs; i++)  {
		xfs_agblock_t		bno = be32_to_cpu(pp[i]);

		if (bno != 0 && verify_agbno(mp, agno, bno)) {
			scan_sbtree(bno, level, agno, suspect, scan_refcbt, 0,
				    magic, priv, &xfs_refcountbt_buf_ops);
		}
	}
out:
	if (suspect)
		refcount_avoid_check();
	return;
}

/*
 * The following helpers are to help process and validate individual on-disk
 * inode btree records. We have two possible inode btrees with slightly
 * different semantics. Many of the validations and actions are equivalent, such
 * as record alignment constraints, etc. Other validations differ, such as the
 * fact that the inode chunk block allocation state is set by the content of the
 * core inobt and verified by the content of the finobt.
 *
 * The following structures are used to facilitate common validation routines
 * where the only difference between validation of the inobt or finobt might be
 * the error messages that results in the event of failure.
 */

enum inobt_type {
	INOBT,
	FINOBT
};
const char *inobt_names[] =	{
	"inobt",
	"finobt"
};

static int
verify_single_ino_chunk_align(
	xfs_agnumber_t		agno,
	enum inobt_type		type,
	struct xfs_inobt_rec	*rp,
	int			suspect,
	bool			*skip)
{
	const char		*inobt_name = inobt_names[type];
	xfs_ino_t		lino;
	xfs_agino_t		ino;
	xfs_agblock_t		agbno;
	int			off;

	*skip = false;
	ino = be32_to_cpu(rp->ir_startino);
	off = XFS_AGINO_TO_OFFSET(mp, ino);
	agbno = XFS_AGINO_TO_AGBNO(mp, ino);
	lino = XFS_AGINO_TO_INO(mp, agno, ino);

	/*
	 * on multi-block block chunks, all chunks start at the beginning of the
	 * block. with multi-chunk blocks, all chunks must start on 64-inode
	 * boundaries since each block can hold N complete chunks. if fs has
	 * aligned inodes, all chunks must start at a fs_ino_alignment*N'th
	 * agbno. skip recs with badly aligned starting inodes.
	 */
	if (ino == 0 ||
	    (inodes_per_block <= XFS_INODES_PER_CHUNK && off !=  0) ||
	    (inodes_per_block > XFS_INODES_PER_CHUNK &&
	     off % XFS_INODES_PER_CHUNK != 0) ||
	    (fs_aligned_inodes && fs_ino_alignment &&
	     agbno % fs_ino_alignment != 0)) {
		do_warn(
	_("badly aligned %s rec (starting inode = %" PRIu64 ")\n"),
			inobt_name, lino);
		suspect++;
	}

	/*
	 * verify numeric validity of inode chunk first before inserting into a
	 * tree. don't have to worry about the overflow case because the
	 * starting ino number of a chunk can only get within 255 inodes of max
	 * (NULLAGINO). if it gets closer, the agino number will be illegal as
	 * the agbno will be too large.
	 */
	if (verify_aginum(mp, agno, ino)) {
		do_warn(
_("bad starting inode # (%" PRIu64 " (0x%x 0x%x)) in %s rec, skipping rec\n"),
			lino, agno, ino, inobt_name);
		*skip = true;
		return ++suspect;
	}

	if (verify_aginum(mp, agno,
			ino + XFS_INODES_PER_CHUNK - 1)) {
		do_warn(
_("bad ending inode # (%" PRIu64 " (0x%x 0x%zx)) in %s rec, skipping rec\n"),
			lino + XFS_INODES_PER_CHUNK - 1,
			agno,
			ino + XFS_INODES_PER_CHUNK - 1,
			inobt_name);
		*skip = true;
		return ++suspect;
	}

	return suspect;
}

/*
 * Process the state of individual inodes in an on-disk inobt record and import
 * into the appropriate in-core tree based on whether the on-disk tree is
 * suspect. Return the total and free inode counts based on the record free and
 * hole masks.
 */
static int
import_single_ino_chunk(
	xfs_agnumber_t		agno,
	enum inobt_type		type,
	struct xfs_inobt_rec	*rp,
	int			suspect,
	int			*p_nfree,
	int			*p_ninodes)
{
	struct ino_tree_node	*ino_rec = NULL;
	const char		*inobt_name = inobt_names[type];
	xfs_agino_t		ino;
	int			j;
	int			nfree;
	int			ninodes;

	ino = be32_to_cpu(rp->ir_startino);

	if (!suspect) {
		if (XFS_INOBT_IS_FREE_DISK(rp, 0))
			ino_rec = set_inode_free_alloc(mp, agno, ino);
		else
			ino_rec = set_inode_used_alloc(mp, agno, ino);
		for (j = 1; j < XFS_INODES_PER_CHUNK; j++) {
			if (XFS_INOBT_IS_FREE_DISK(rp, j))
				set_inode_free(ino_rec, j);
			else
				set_inode_used(ino_rec, j);
		}
	} else {
		for (j = 0; j < XFS_INODES_PER_CHUNK; j++) {
			if (XFS_INOBT_IS_FREE_DISK(rp, j))
				add_aginode_uncertain(mp, agno, ino + j, 1);
			else
				add_aginode_uncertain(mp, agno, ino + j, 0);
		}
	}

	/*
	 * Mark sparse inodes as such in the in-core tree. Verify that sparse
	 * inodes are free and that freecount is consistent with the free mask.
	 */
	nfree = ninodes = 0;
	for (j = 0; j < XFS_INODES_PER_CHUNK; j++) {
		if (ino_issparse(rp, j)) {
			if (!suspect && !XFS_INOBT_IS_FREE_DISK(rp, j)) {
				do_warn(
_("ir_holemask/ir_free mismatch, %s chunk %d/%u, holemask 0x%x free 0x%llx\n"),
					inobt_name, agno, ino,
					be16_to_cpu(rp->ir_u.sp.ir_holemask),
					(unsigned long long)be64_to_cpu(rp->ir_free));
				suspect++;
			}
			if (!suspect && ino_rec)
				set_inode_sparse(ino_rec, j);
		} else {
			/* count fields track non-sparse inos */
			if (XFS_INOBT_IS_FREE_DISK(rp, j))
				nfree++;
			ninodes++;
		}
	}

	*p_nfree = nfree;
	*p_ninodes = ninodes;

	return suspect;
}

static int
scan_single_ino_chunk(
	xfs_agnumber_t		agno,
	xfs_inobt_rec_t		*rp,
	int			suspect)
{
	xfs_ino_t		lino;
	xfs_agino_t		ino;
	xfs_agblock_t		agbno;
	int			j;
	int			nfree;
	int			ninodes;
	int			off;
	int			state;
	ino_tree_node_t		*first_rec, *last_rec;
	int			freecount;
	bool			skip = false;

	ino = be32_to_cpu(rp->ir_startino);
	off = XFS_AGINO_TO_OFFSET(mp, ino);
	agbno = XFS_AGINO_TO_AGBNO(mp, ino);
	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	freecount = inorec_get_freecount(mp, rp);

	/*
	 * Verify record alignment, start/end inode numbers, etc.
	 */
	suspect = verify_single_ino_chunk_align(agno, INOBT, rp, suspect,
						&skip);
	if (skip)
		return suspect;

	/*
	 * set state of each block containing inodes
	 */
	if (off == 0 && !suspect)  {
		for (j = 0;
		     j < XFS_INODES_PER_CHUNK;
		     j += mp->m_sb.sb_inopblock)  {

			/* inodes in sparse chunks don't use blocks */
			if (ino_issparse(rp, j))
				continue;

			agbno = XFS_AGINO_TO_AGBNO(mp, ino + j);
			state = get_bmap(agno, agbno);
			switch (state) {
			case XR_E_INO:
				break;
			case XR_E_UNKNOWN:
			case XR_E_INO1:	/* seen by rmap */
				set_bmap(agno, agbno, XR_E_INO);
				break;
			case XR_E_INUSE_FS:
			case XR_E_INUSE_FS1:
				if (agno == 0 &&
				    ino + j >= first_prealloc_ino &&
				    ino + j < last_prealloc_ino) {
					set_bmap(agno, agbno, XR_E_INO);
					break;
				}
				/* fall through */
			default:
				/* XXX - maybe should mark block a duplicate */
				do_warn(
_("inode chunk claims used block, inobt block - agno %d, bno %d, inopb %d\n"),
					agno, agbno, mp->m_sb.sb_inopblock);
				return ++suspect;
			}
		}
	}

	/*
	 * ensure only one avl entry per chunk
	 */
	find_inode_rec_range(mp, agno, ino, ino + XFS_INODES_PER_CHUNK,
			     &first_rec, &last_rec);
	if (first_rec != NULL)  {
		/*
		 * this chunk overlaps with one (or more)
		 * already in the tree
		 */
		do_warn(
_("inode rec for ino %" PRIu64 " (%d/%d) overlaps existing rec (start %d/%d)\n"),
			lino, agno, ino, agno, first_rec->ino_startnum);
		suspect++;

		/*
		 * if the 2 chunks start at the same place,
		 * then we don't have to put this one
		 * in the uncertain list.  go to the next one.
		 */
		if (first_rec->ino_startnum == ino)
			return suspect;
	}

	/*
	 * Import the state of individual inodes into the appropriate in-core
	 * trees, mark them free or used, and get the resulting total and free
	 * inode counts.
	 */
	nfree = ninodes = 0;
	suspect = import_single_ino_chunk(agno, INOBT, rp, suspect, &nfree,
					 &ninodes);

	if (nfree != freecount) {
		do_warn(
_("ir_freecount/free mismatch, inode chunk %d/%u, freecount %d nfree %d\n"),
			agno, ino, freecount, nfree);
	}

	/* verify sparse record formats have a valid inode count */
	if (xfs_sb_version_hassparseinodes(&mp->m_sb) &&
	    ninodes != rp->ir_u.sp.ir_count) {
		do_warn(
_("invalid inode count, inode chunk %d/%u, count %d ninodes %d\n"),
			agno, ino, rp->ir_u.sp.ir_count, ninodes);
	}

	return suspect;
}

static int
scan_single_finobt_chunk(
	xfs_agnumber_t		agno,
	xfs_inobt_rec_t		*rp,
	int			suspect)
{
	xfs_ino_t		lino;
	xfs_agino_t		ino;
	xfs_agblock_t		agbno;
	int			j;
	int			nfree;
	int			ninodes;
	int			off;
	int			state;
	ino_tree_node_t		*first_rec, *last_rec;
	int			freecount;
	bool			skip = false;

	ino = be32_to_cpu(rp->ir_startino);
	off = XFS_AGINO_TO_OFFSET(mp, ino);
	agbno = XFS_AGINO_TO_AGBNO(mp, ino);
	lino = XFS_AGINO_TO_INO(mp, agno, ino);
	freecount = inorec_get_freecount(mp, rp);

	/*
	 * Verify record alignment, start/end inode numbers, etc.
	 */
	suspect = verify_single_ino_chunk_align(agno, FINOBT, rp, suspect,
						&skip);
	if (skip)
		return suspect;

	/*
	 * cross check state of each block containing inodes referenced by the
	 * finobt against what we have already scanned from the alloc inobt.
	 */
	if (off == 0 && !suspect) {
		for (j = 0;
		     j < XFS_INODES_PER_CHUNK;
		     j += mp->m_sb.sb_inopblock) {
			agbno = XFS_AGINO_TO_AGBNO(mp, ino + j);
			state = get_bmap(agno, agbno);

			/* sparse inodes should not refer to inode blocks */
			if (ino_issparse(rp, j)) {
				if (state == XR_E_INO) {
					do_warn(
_("sparse inode chunk claims inode block, finobt block - agno %d, bno %d, inopb %d\n"),
						agno, agbno, mp->m_sb.sb_inopblock);
					suspect++;
				}
				continue;
			}

			switch (state) {
			case XR_E_INO:
				break;
			case XR_E_INO1:	/* seen by rmap */
				set_bmap(agno, agbno, XR_E_INO);
				break;
			case XR_E_UNKNOWN:
				do_warn(
_("inode chunk claims untracked block, finobt block - agno %d, bno %d, inopb %d\n"),
					agno, agbno, mp->m_sb.sb_inopblock);

				set_bmap(agno, agbno, XR_E_INO);
				suspect++;
				break;
			case XR_E_INUSE_FS:
			case XR_E_INUSE_FS1:
				if (agno == 0 &&
				    ino + j >= first_prealloc_ino &&
				    ino + j < last_prealloc_ino) {
					do_warn(
_("inode chunk claims untracked block, finobt block - agno %d, bno %d, inopb %d\n"),
						agno, agbno, mp->m_sb.sb_inopblock);

					set_bmap(agno, agbno, XR_E_INO);
					suspect++;
					break;
				}
				/* fall through */
			default:
				do_warn(
_("inode chunk claims used block, finobt block - agno %d, bno %d, inopb %d\n"),
					agno, agbno, mp->m_sb.sb_inopblock);
				return ++suspect;
			}
		}
	}

	/*
	 * ensure we have an incore entry for each chunk
	 */
	find_inode_rec_range(mp, agno, ino, ino + XFS_INODES_PER_CHUNK,
			     &first_rec, &last_rec);

	if (first_rec) {
		if (suspect)
			return suspect;

		/*
		 * verify consistency between finobt record and incore state
		 */
		if (first_rec->ino_startnum != ino) {
			do_warn(
_("finobt rec for ino %" PRIu64 " (%d/%u) does not match existing rec (%d/%d)\n"),
				lino, agno, ino, agno, first_rec->ino_startnum);
			return ++suspect;
		}

		nfree = ninodes = 0;
		for (j = 0; j < XFS_INODES_PER_CHUNK; j++) {
			int isfree = XFS_INOBT_IS_FREE_DISK(rp, j);
			int issparse = ino_issparse(rp, j);

			if (!issparse)
				ninodes++;
			if (isfree && !issparse)
				nfree++;

			/*
			 * inode allocation state should be consistent between
			 * the inobt and finobt
			 */
			if (!suspect &&
			    isfree != is_inode_free(first_rec, j))
				suspect++;

			if (!suspect &&
			    issparse != is_inode_sparse(first_rec, j))
				suspect++;
		}

		goto check_freecount;
	}

	/*
	 * The finobt contains a record that the previous inobt scan never
	 * found. Warn about it and import the inodes into the appropriate
	 * trees.
	 *
	 * Note that this should do the right thing if the previous inobt scan
	 * had added these inodes to the uncertain tree. If the finobt is not
	 * suspect, these inodes should supercede the uncertain ones. Otherwise,
	 * the uncertain tree helpers handle the case where uncertain inodes
	 * already exist.
	 */
	do_warn(_("undiscovered finobt record, ino %" PRIu64 " (%d/%u)\n"),
		lino, agno, ino);

	nfree = ninodes = 0;
	suspect = import_single_ino_chunk(agno, FINOBT, rp, suspect, &nfree,
					 &ninodes);

check_freecount:

	/*
	 * Verify that the record freecount matches the actual number of free
	 * inodes counted in the record. Don't increment 'suspect' here, since
	 * we have already verified the allocation state of the individual
	 * inodes against the in-core state. This will have already incremented
	 * 'suspect' if something is wrong. If suspect hasn't been set at this
	 * point, these warnings mean that we have a simple freecount
	 * inconsistency or a stray finobt record (as opposed to a broader tree
	 * corruption). Issue a warning and continue the scan. The final btree
	 * reconstruction will correct this naturally.
	 */
	if (nfree != freecount) {
		do_warn(
_("finobt ir_freecount/free mismatch, inode chunk %d/%u, freecount %d nfree %d\n"),
			agno, ino, freecount, nfree);
	}

	if (!nfree) {
		do_warn(
_("finobt record with no free inodes, inode chunk %d/%u\n"), agno, ino);
	}

	/* verify sparse record formats have a valid inode count */
	if (xfs_sb_version_hassparseinodes(&mp->m_sb) &&
	    ninodes != rp->ir_u.sp.ir_count) {
		do_warn(
_("invalid inode count, inode chunk %d/%u, count %d ninodes %d\n"),
			agno, ino, rp->ir_u.sp.ir_count, ninodes);
	}

	return suspect;
}

/*
 * this one walks the inode btrees sucking the info there into
 * the incore avl tree.  We try and rescue corrupted btree records
 * to minimize our chances of losing inodes.  Inode info from potentially
 * corrupt sources could be bogus so rather than put the info straight
 * into the tree, instead we put it on a list and try and verify the
 * info in the next phase by examining what's on disk.  At that point,
 * we'll be able to figure out what's what and stick the corrected info
 * into the tree.  We do bail out at some point and give up on a subtree
 * so as to avoid walking randomly all over the ag.
 *
 * Note that it's also ok if the free/inuse info wrong, we can correct
 * that when we examine the on-disk inode.  The important thing is to
 * get the start and alignment of the inode chunks right.  Those chunks
 * that we aren't sure about go into the uncertain list.
 */
static void
scan_inobt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	uint32_t		magic,
	void			*priv)
{
	struct aghdr_cnts	*agcnts = priv;
	int			i;
	int			numrecs;
	int			state;
	xfs_inobt_ptr_t		*pp;
	xfs_inobt_rec_t		*rp;
	int			hdr_errors;
	int			freecount;

	hdr_errors = 0;

	if (be32_to_cpu(block->bb_magic) != magic) {
		do_warn(_("bad magic # %#x in inobt block %d/%d\n"),
			be32_to_cpu(block->bb_magic), agno, bno);
		hdr_errors++;
		bad_ino_btree = 1;
		if (suspect)
			return;
	}
	if (be16_to_cpu(block->bb_level) != level) {
		do_warn(_("expected level %d got %d in inobt block %d/%d\n"),
			level, be16_to_cpu(block->bb_level), agno, bno);
		hdr_errors++;
		bad_ino_btree = 1;
		if (suspect)
			return;
	}

	/*
	 * check for btree blocks multiply claimed, any unknown/free state
	 * is ok in the bitmap block.
	 */
	state = get_bmap(agno, bno);
	switch (state)  {
	case XR_E_FS_MAP1: /* already been seen by an rmap scan */
	case XR_E_UNKNOWN:
	case XR_E_FREE1:
	case XR_E_FREE:
		set_bmap(agno, bno, XR_E_FS_MAP);
		break;
	default:
		set_bmap(agno, bno, XR_E_MULT);
		do_warn(
_("inode btree block claimed (state %d), agno %d, bno %d, suspect %d\n"),
			state, agno, bno, suspect);
	}

	numrecs = be16_to_cpu(block->bb_numrecs);

	/*
	 * leaf record in btree
	 */
	if (level == 0) {
		/* check for trashed btree block */

		if (numrecs > mp->m_inobt_mxr[0])  {
			numrecs = mp->m_inobt_mxr[0];
			hdr_errors++;
		}
		if (isroot == 0 && numrecs < mp->m_inobt_mnr[0])  {
			numrecs = mp->m_inobt_mnr[0];
			hdr_errors++;
		}

		if (hdr_errors)  {
			bad_ino_btree = 1;
			do_warn(_("dubious inode btree block header %d/%d\n"),
				agno, bno);
			suspect++;
		}

		rp = XFS_INOBT_REC_ADDR(mp, block, 1);

		/*
		 * step through the records, each record points to
		 * a chunk of inodes.  The start of inode chunks should
		 * be block-aligned.  Each inode btree rec should point
		 * to the start of a block of inodes or the start of a group
		 * of INODES_PER_CHUNK (64) inodes.  off is the offset into
		 * the block.  skip processing of bogus records.
		 */
		for (i = 0; i < numrecs; i++) {
			freecount = inorec_get_freecount(mp, &rp[i]);

			if (magic == XFS_IBT_MAGIC ||
			    magic == XFS_IBT_CRC_MAGIC) {
				int icount = XFS_INODES_PER_CHUNK;

				/*
				 * ir_count holds the inode count for all
				 * records on fs' with sparse inode support
				 */
				if (xfs_sb_version_hassparseinodes(&mp->m_sb))
					icount = rp[i].ir_u.sp.ir_count;

				agcnts->agicount += icount;
				agcnts->agifreecount += freecount;
				agcnts->ifreecount += freecount;

				suspect = scan_single_ino_chunk(agno, &rp[i],
						suspect);
			} else {
				/*
				 * the finobt tracks records with free inodes,
				 * so only the free inode count is expected to be
				 * consistent with the agi
				 */
				agcnts->fibtfreecount += freecount;

				suspect = scan_single_finobt_chunk(agno, &rp[i],
						suspect);
			}
		}

		if (suspect)
			bad_ino_btree = 1;

		return;
	}

	/*
	 * interior record, continue on
	 */
	if (numrecs > mp->m_inobt_mxr[1])  {
		numrecs = mp->m_inobt_mxr[1];
		hdr_errors++;
	}
	if (isroot == 0 && numrecs < mp->m_inobt_mnr[1])  {
		numrecs = mp->m_inobt_mnr[1];
		hdr_errors++;
	}

	pp = XFS_INOBT_PTR_ADDR(mp, block, 1, mp->m_inobt_mxr[1]);

	/*
	 * don't pass bogus tree flag down further if this block
	 * looked ok.  bail out if two levels in a row look bad.
	 */

	if (suspect && !hdr_errors)
		suspect = 0;

	if (hdr_errors)  {
		bad_ino_btree = 1;
		if (suspect)
			return;
		else suspect++;
	}

	for (i = 0; i < numrecs; i++)  {
		if (be32_to_cpu(pp[i]) != 0 && verify_agbno(mp, agno,
							be32_to_cpu(pp[i])))
			scan_sbtree(be32_to_cpu(pp[i]), level, agno,
					suspect, scan_inobt, 0, magic, priv,
					&xfs_inobt_buf_ops);
	}
}

static void
scan_freelist(
	xfs_agf_t	*agf,
	struct aghdr_cnts *agcnts)
{
	xfs_buf_t	*agflbuf;
	xfs_agnumber_t	agno;
	xfs_agblock_t	bno;
	int		count;
	int		i;
	__be32		*freelist;

	agno = be32_to_cpu(agf->agf_seqno);

	if (XFS_SB_BLOCK(mp) != XFS_AGFL_BLOCK(mp) &&
	    XFS_AGF_BLOCK(mp) != XFS_AGFL_BLOCK(mp) &&
	    XFS_AGI_BLOCK(mp) != XFS_AGFL_BLOCK(mp))
		set_bmap(agno, XFS_AGFL_BLOCK(mp), XR_E_INUSE_FS);

	if (be32_to_cpu(agf->agf_flcount) == 0)
		return;

	agflbuf = libxfs_readbuf(mp->m_dev,
				 XFS_AG_DADDR(mp, agno, XFS_AGFL_DADDR(mp)),
				 XFS_FSS_TO_BB(mp, 1), 0, &xfs_agfl_buf_ops);
	if (!agflbuf)  {
		do_abort(_("can't read agfl block for ag %d\n"), agno);
		return;
	}
	if (agflbuf->b_error == -EFSBADCRC)
		do_warn(_("agfl has bad CRC for ag %d\n"), agno);

	freelist = XFS_BUF_TO_AGFL_BNO(mp, agflbuf);
	i = be32_to_cpu(agf->agf_flfirst);

	if (no_modify) {
		/* agf values not fixed in verify_set_agf, so recheck */
		if (be32_to_cpu(agf->agf_flfirst) >= XFS_AGFL_SIZE(mp) ||
		    be32_to_cpu(agf->agf_fllast) >= XFS_AGFL_SIZE(mp)) {
			do_warn(_("agf %d freelist blocks bad, skipping "
				  "freelist scan\n"), i);
			return;
		}
	}

	count = 0;
	for (;;) {
		bno = be32_to_cpu(freelist[i]);
		if (verify_agbno(mp, agno, bno))
			set_bmap(agno, bno, XR_E_FREE);
		else
			do_warn(_("bad agbno %u in agfl, agno %d\n"),
				bno, agno);
		count++;
		if (i == be32_to_cpu(agf->agf_fllast))
			break;
		if (++i == XFS_AGFL_SIZE(mp))
			i = 0;
	}
	if (count != be32_to_cpu(agf->agf_flcount)) {
		do_warn(_("freeblk count %d != flcount %d in ag %d\n"), count,
			be32_to_cpu(agf->agf_flcount), agno);
	}

	agcnts->fdblocks += count;

	libxfs_putbuf(agflbuf);
}

static void
validate_agf(
	struct xfs_agf		*agf,
	xfs_agnumber_t		agno,
	struct aghdr_cnts	*agcnts)
{
	xfs_agblock_t		bno;
	uint32_t		magic;

	bno = be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNO]);
	if (bno != 0 && verify_agbno(mp, agno, bno)) {
		magic = xfs_sb_version_hascrc(&mp->m_sb) ? XFS_ABTB_CRC_MAGIC
							 : XFS_ABTB_MAGIC;
		scan_sbtree(bno, be32_to_cpu(agf->agf_levels[XFS_BTNUM_BNO]),
			    agno, 0, scan_allocbt, 1, magic, agcnts,
			    &xfs_allocbt_buf_ops);
	} else {
		do_warn(_("bad agbno %u for btbno root, agno %d\n"),
			bno, agno);
	}

	bno = be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNT]);
	if (bno != 0 && verify_agbno(mp, agno, bno)) {
		magic = xfs_sb_version_hascrc(&mp->m_sb) ? XFS_ABTC_CRC_MAGIC
							 : XFS_ABTC_MAGIC;
		scan_sbtree(bno, be32_to_cpu(agf->agf_levels[XFS_BTNUM_CNT]),
			    agno, 0, scan_allocbt, 1, magic, agcnts,
			    &xfs_allocbt_buf_ops);
	} else  {
		do_warn(_("bad agbno %u for btbcnt root, agno %d\n"),
			bno, agno);
	}

	if (xfs_sb_version_hasrmapbt(&mp->m_sb)) {
		struct rmap_priv	priv;

		memset(&priv.high_key, 0xFF, sizeof(priv.high_key));
		priv.high_key.rm_blockcount = 0;
		priv.agcnts = agcnts;
		priv.last_rec.rm_owner = XFS_RMAP_OWN_UNKNOWN;
		priv.nr_blocks = 0;
		bno = be32_to_cpu(agf->agf_roots[XFS_BTNUM_RMAP]);
		if (bno != 0 && verify_agbno(mp, agno, bno)) {
			scan_sbtree(bno,
				    be32_to_cpu(agf->agf_levels[XFS_BTNUM_RMAP]),
				    agno, 0, scan_rmapbt, 1, XFS_RMAP_CRC_MAGIC,
				    &priv, &xfs_rmapbt_buf_ops);
			if (be32_to_cpu(agf->agf_rmap_blocks) != priv.nr_blocks)
				do_warn(_("bad rmapbt block count %u, saw %u\n"),
					priv.nr_blocks,
					be32_to_cpu(agf->agf_rmap_blocks));
		} else  {
			do_warn(_("bad agbno %u for rmapbt root, agno %d\n"),
				bno, agno);
			rmap_avoid_check();
		}
	}

	if (xfs_sb_version_hasreflink(&mp->m_sb)) {
		bno = be32_to_cpu(agf->agf_refcount_root);
		if (bno != 0 && verify_agbno(mp, agno, bno)) {
			struct refc_priv	priv;

			memset(&priv, 0, sizeof(priv));
			scan_sbtree(bno,
				    be32_to_cpu(agf->agf_refcount_level),
				    agno, 0, scan_refcbt, 1, XFS_REFC_CRC_MAGIC,
				    &priv, &xfs_refcountbt_buf_ops);
			if (be32_to_cpu(agf->agf_refcount_blocks) != priv.nr_blocks)
				do_warn(_("bad refcountbt block count %u, saw %u\n"),
					priv.nr_blocks,
					be32_to_cpu(agf->agf_refcount_blocks));
		} else  {
			do_warn(_("bad agbno %u for refcntbt root, agno %d\n"),
				bno, agno);
			refcount_avoid_check();
		}
	}

	if (be32_to_cpu(agf->agf_freeblks) != agcnts->agffreeblks) {
		do_warn(_("agf_freeblks %u, counted %u in ag %u\n"),
			be32_to_cpu(agf->agf_freeblks), agcnts->agffreeblks, agno);
	}

	if (be32_to_cpu(agf->agf_longest) != agcnts->agflongest) {
		do_warn(_("agf_longest %u, counted %u in ag %u\n"),
			be32_to_cpu(agf->agf_longest), agcnts->agflongest, agno);
	}

	if (xfs_sb_version_haslazysbcount(&mp->m_sb) &&
	    be32_to_cpu(agf->agf_btreeblks) != agcnts->agfbtreeblks) {
		do_warn(_("agf_btreeblks %u, counted %" PRIu64 " in ag %u\n"),
			be32_to_cpu(agf->agf_btreeblks), agcnts->agfbtreeblks, agno);
	}

}

static void
validate_agi(
	struct xfs_agi		*agi,
	xfs_agnumber_t		agno,
	struct aghdr_cnts	*agcnts)
{
	xfs_agblock_t		bno;
	int			i;
	uint32_t		magic;

	bno = be32_to_cpu(agi->agi_root);
	if (bno != 0 && verify_agbno(mp, agno, bno)) {
		magic = xfs_sb_version_hascrc(&mp->m_sb) ? XFS_IBT_CRC_MAGIC
							 : XFS_IBT_MAGIC;
		scan_sbtree(bno, be32_to_cpu(agi->agi_level),
			    agno, 0, scan_inobt, 1, magic, agcnts,
			    &xfs_inobt_buf_ops);
	} else {
		do_warn(_("bad agbno %u for inobt root, agno %d\n"),
			be32_to_cpu(agi->agi_root), agno);
	}

	if (xfs_sb_version_hasfinobt(&mp->m_sb)) {
		bno = be32_to_cpu(agi->agi_free_root);
		if (bno != 0 && verify_agbno(mp, agno, bno)) {
			magic = xfs_sb_version_hascrc(&mp->m_sb) ?
					XFS_FIBT_CRC_MAGIC : XFS_FIBT_MAGIC;
			scan_sbtree(bno, be32_to_cpu(agi->agi_free_level),
				    agno, 0, scan_inobt, 1, magic, agcnts,
				    &xfs_inobt_buf_ops);
		} else {
			do_warn(_("bad agbno %u for finobt root, agno %d\n"),
				be32_to_cpu(agi->agi_free_root), agno);
		}
	}

	if (be32_to_cpu(agi->agi_count) != agcnts->agicount) {
		do_warn(_("agi_count %u, counted %u in ag %u\n"),
			 be32_to_cpu(agi->agi_count), agcnts->agicount, agno);
	}

	if (be32_to_cpu(agi->agi_freecount) != agcnts->agifreecount) {
		do_warn(_("agi_freecount %u, counted %u in ag %u\n"),
			be32_to_cpu(agi->agi_freecount), agcnts->agifreecount, agno);
	}

	if (xfs_sb_version_hasfinobt(&mp->m_sb) &&
	    be32_to_cpu(agi->agi_freecount) != agcnts->fibtfreecount) {
		do_warn(_("agi_freecount %u, counted %u in ag %u finobt\n"),
			be32_to_cpu(agi->agi_freecount), agcnts->fibtfreecount,
			agno);
	}

	for (i = 0; i < XFS_AGI_UNLINKED_BUCKETS; i++) {
		xfs_agino_t	agino = be32_to_cpu(agi->agi_unlinked[i]);

		if (agino != NULLAGINO) {
			do_warn(
	_("agi unlinked bucket %d is %u in ag %u (inode=%" PRIu64 ")\n"),
				i, agino, agno,
				XFS_AGINO_TO_INO(mp, agno, agino));
		}
	}
}

/*
 * Scan an AG for obvious corruption.
 */
static void
scan_ag(
	work_queue_t	*wq,
	xfs_agnumber_t	agno,
	void		*arg)
{
	struct aghdr_cnts *agcnts = arg;
	struct xfs_agf	*agf;
	struct xfs_buf	*agfbuf = NULL;
	int		agf_dirty = 0;
	struct xfs_agi	*agi;
	struct xfs_buf	*agibuf = NULL;
	int		agi_dirty = 0;
	struct xfs_sb	*sb = NULL;
	struct xfs_buf	*sbbuf = NULL;
	int		sb_dirty = 0;
	int		status;
	char		*objname = NULL;

	sb = (struct xfs_sb *)calloc(BBTOB(XFS_FSS_TO_BB(mp, 1)), 1);
	if (!sb) {
		do_error(_("can't allocate memory for superblock\n"));
		return;
	}

	sbbuf = libxfs_readbuf(mp->m_dev, XFS_AG_DADDR(mp, agno, XFS_SB_DADDR),
				XFS_FSS_TO_BB(mp, 1), 0, &xfs_sb_buf_ops);
	if (!sbbuf)  {
		objname = _("root superblock");
		goto out_free_sb;
	}
	libxfs_sb_from_disk(sb, XFS_BUF_TO_SBP(sbbuf));

	agfbuf = libxfs_readbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), 0, &xfs_agf_buf_ops);
	if (!agfbuf)  {
		objname = _("agf block");
		goto out_free_sbbuf;
	}
	agf = XFS_BUF_TO_AGF(agfbuf);

	agibuf = libxfs_readbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), 0, &xfs_agi_buf_ops);
	if (!agibuf)  {
		objname = _("agi block");
		goto out_free_agfbuf;
	}
	agi = XFS_BUF_TO_AGI(agibuf);

	/* fix up bad ag headers */

	status = verify_set_agheader(mp, sbbuf, sb, agf, agi, agno);

	if (status & XR_AG_SB_SEC)  {
		if (!no_modify)
			sb_dirty = 1;
		/*
		 * clear bad sector bit because we don't want
		 * to skip further processing.  we just want to
		 * ensure that we write out the modified sb buffer.
		 */
		status &= ~XR_AG_SB_SEC;
	}
	if (status & XR_AG_SB)  {
		if (!no_modify) {
			do_warn(_("reset bad sb for ag %d\n"), agno);
			sb_dirty = 1;
		} else {
			do_warn(_("would reset bad sb for ag %d\n"), agno);
		}
	}
	if (status & XR_AG_AGF)  {
		if (!no_modify) {
			do_warn(_("reset bad agf for ag %d\n"), agno);
			agf_dirty = 1;
		} else {
			do_warn(_("would reset bad agf for ag %d\n"), agno);
		}
	}
	if (status & XR_AG_AGI)  {
		if (!no_modify) {
			do_warn(_("reset bad agi for ag %d\n"), agno);
			agi_dirty = 1;
		} else {
			do_warn(_("would reset bad agi for ag %d\n"), agno);
		}
	}

	if (status && no_modify)  {
		do_warn(_("bad uncorrected agheader %d, skipping ag...\n"),
			agno);
		goto out_free_agibuf;
	}

	scan_freelist(agf, agcnts);

	validate_agf(agf, agno, agcnts);
	validate_agi(agi, agno, agcnts);

	ASSERT(agi_dirty == 0 || (agi_dirty && !no_modify));
	ASSERT(agf_dirty == 0 || (agf_dirty && !no_modify));
	ASSERT(sb_dirty == 0 || (sb_dirty && !no_modify));

	/*
	 * Only pay attention to CRC/verifier errors if we can correct them.
	 * Note that we can get uncorrected EFSCORRUPTED errors here because
	 * the verifier will flag on out of range values that we can't correct
	 * until phase 5 when we have all the information necessary to rebuild
	 * the freespace/inode btrees. We can correct bad CRC errors
	 * immediately, though.
	 */
	if (!no_modify) {
		agi_dirty += (agibuf->b_error == -EFSBADCRC);
		agf_dirty += (agfbuf->b_error == -EFSBADCRC);
		sb_dirty += (sbbuf->b_error == -EFSBADCRC);
	}

	if (agi_dirty && !no_modify)
		libxfs_writebuf(agibuf, 0);
	else
		libxfs_putbuf(agibuf);

	if (agf_dirty && !no_modify)
		libxfs_writebuf(agfbuf, 0);
	else
		libxfs_putbuf(agfbuf);

	if (sb_dirty && !no_modify) {
		if (agno == 0)
			memcpy(&mp->m_sb, sb, sizeof(xfs_sb_t));
		libxfs_sb_to_disk(XFS_BUF_TO_SBP(sbbuf), sb);
		libxfs_writebuf(sbbuf, 0);
	} else
		libxfs_putbuf(sbbuf);
	free(sb);
	PROG_RPT_INC(prog_rpt_done[agno], 1);

#ifdef XR_INODE_TRACE
	print_inode_list(i);
#endif
	return;

out_free_agibuf:
	libxfs_putbuf(agibuf);
out_free_agfbuf:
	libxfs_putbuf(agfbuf);
out_free_sbbuf:
	libxfs_putbuf(sbbuf);
out_free_sb:
	free(sb);

	if (objname)
		do_error(_("can't get %s for ag %d\n"), objname, agno);
}

#define SCAN_THREADS 32

void
scan_ags(
	struct xfs_mount	*mp,
	int			scan_threads)
{
	struct aghdr_cnts *agcnts;
	uint64_t	fdblocks = 0;
	uint64_t	icount = 0;
	uint64_t	ifreecount = 0;
	uint64_t	usedblocks = 0;
	xfs_agnumber_t	i;
	work_queue_t	wq;

	agcnts = malloc(mp->m_sb.sb_agcount * sizeof(*agcnts));
	if (!agcnts) {
		do_abort(_("no memory for ag header counts\n"));
		return;
	}
	memset(agcnts, 0, mp->m_sb.sb_agcount * sizeof(*agcnts));

	create_work_queue(&wq, mp, scan_threads);

	for (i = 0; i < mp->m_sb.sb_agcount; i++)
		queue_work(&wq, scan_ag, i, &agcnts[i]);

	destroy_work_queue(&wq);

	/* tally up the counts */
	for (i = 0; i < mp->m_sb.sb_agcount; i++) {
		fdblocks += agcnts[i].fdblocks;
		icount += agcnts[i].agicount;
		ifreecount += agcnts[i].ifreecount;
		usedblocks += agcnts[i].usedblocks;
	}

	free(agcnts);

	/*
	 * Validate that our manual counts match the superblock.
	 */
	if (mp->m_sb.sb_icount != icount) {
		do_warn(_("sb_icount %" PRIu64 ", counted %" PRIu64 "\n"),
			mp->m_sb.sb_icount, icount);
	}

	if (mp->m_sb.sb_ifree != ifreecount) {
		do_warn(_("sb_ifree %" PRIu64 ", counted %" PRIu64 "\n"),
			mp->m_sb.sb_ifree, ifreecount);
	}

	if (mp->m_sb.sb_fdblocks != fdblocks) {
		do_warn(_("sb_fdblocks %" PRIu64 ", counted %" PRIu64 "\n"),
			mp->m_sb.sb_fdblocks, fdblocks);
	}

	if (usedblocks &&
	    usedblocks != mp->m_sb.sb_dblocks - fdblocks) {
		do_warn(_("used blocks %" PRIu64 ", counted %" PRIu64 "\n"),
			mp->m_sb.sb_dblocks - fdblocks, usedblocks);
	}
}
