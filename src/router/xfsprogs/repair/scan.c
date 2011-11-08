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

#include <libxfs.h>
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

extern int verify_set_agheader(xfs_mount_t *mp, xfs_buf_t *sbuf, xfs_sb_t *sb,
		xfs_agf_t *agf, xfs_agi_t *agi, xfs_agnumber_t i);

static xfs_mount_t	*mp = NULL;

/*
 * Variables to validate AG header values against the manual count
 * from the btree traversal.
 */
struct aghdr_cnts {
	xfs_agnumber_t	agno;
	xfs_extlen_t	agffreeblks;
	xfs_extlen_t	agflongest;
	__uint64_t	agfbtreeblks;
	__uint32_t	agicount;
	__uint32_t	agifreecount;
	__uint64_t	fdblocks;
	__uint64_t	icount;
	__uint64_t	ifreecount;
};

static void
scanfunc_allocbt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	__uint32_t		magic,
	struct aghdr_cnts	*agcnts);

void
set_mp(xfs_mount_t *mpp)
{
	libxfs_bcache_purge();
	mp = mpp;
}

void
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
				void			*priv),
	int		isroot,
	void		*priv)
{
	xfs_buf_t	*bp;

	bp = libxfs_readbuf(mp->m_dev, XFS_AGB_TO_DADDR(mp, agno, root),
			XFS_FSB_TO_BB(mp, 1), 0);
	if (!bp) {
		do_error(_("can't read btree block %d/%d\n"), agno, root);
		return;
	}
	(*func)(XFS_BUF_TO_BLOCK(bp), nlevels - 1, root, agno, suspect,
							isroot, priv);
	libxfs_putbuf(bp);
}

/*
 * returns 1 on bad news (inode needs to be cleared), 0 on good
 */
int
scan_lbtree(
	xfs_dfsbno_t	root,
	int		nlevels,
	int		(*func)(struct xfs_btree_block	*block,
				int			level,
				int			type,
				int			whichfork,
				xfs_dfsbno_t		bno,
				xfs_ino_t		ino,
				xfs_drfsbno_t		*tot,
				__uint64_t		*nex,
				blkmap_t		**blkmapp,
				bmap_cursor_t		*bm_cursor,
				int			isroot,
				int			check_dups,
				int			*dirty),
	int		type,
	int		whichfork,
	xfs_ino_t	ino,
	xfs_drfsbno_t	*tot,
	__uint64_t	*nex,
	blkmap_t	**blkmapp,
	bmap_cursor_t	*bm_cursor,
	int		isroot,
	int		check_dups)
{
	xfs_buf_t	*bp;
	int		err;
	int		dirty = 0;

	bp = libxfs_readbuf(mp->m_dev, XFS_FSB_TO_DADDR(mp, root),
		      XFS_FSB_TO_BB(mp, 1), 0);
	if (!bp)  {
		do_error(_("can't read btree block %d/%d\n"),
			XFS_FSB_TO_AGNO(mp, root),
			XFS_FSB_TO_AGBNO(mp, root));
		return(1);
	}
	err = (*func)(XFS_BUF_TO_BLOCK(bp), nlevels - 1,
			type, whichfork, root, ino, tot, nex, blkmapp,
			bm_cursor, isroot, check_dups, &dirty);

	ASSERT(dirty == 0 || (dirty && !no_modify));

	if (dirty && !no_modify)
		libxfs_writebuf(bp, 0);
	else
		libxfs_putbuf(bp);

	return(err);
}

int
scanfunc_bmap(
	struct xfs_btree_block	*block,
	int			level,
	int			type,
	int			whichfork,
	xfs_dfsbno_t		bno,
	xfs_ino_t		ino,
	xfs_drfsbno_t		*tot,
	__uint64_t		*nex,
	blkmap_t		**blkmapp,
	bmap_cursor_t		*bm_cursor,
	int			isroot,
	int			check_dups,
	int			*dirty)
{
	int			i;
	int			err;
	xfs_bmbt_ptr_t		*pp;
	xfs_bmbt_key_t		*pkey;
	xfs_bmbt_rec_t		*rp;
	xfs_dfiloff_t		first_key;
	xfs_dfiloff_t		last_key;
	char			*forkname;
	int			numrecs;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	int			state;

	if (whichfork == XFS_DATA_FORK)
		forkname = _("data");
	else
		forkname = _("attr");

	/*
	 * unlike the ag freeblock btrees, if anything looks wrong
	 * in an inode bmap tree, just bail.  it's possible that
	 * we'll miss a case where the to-be-toasted inode and
	 * another inode are claiming the same block but that's
	 * highly unlikely.
	 */
	if (be32_to_cpu(block->bb_magic) != XFS_BMAP_MAGIC) {
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

	if (check_dups == 0)  {
		/*
		 * check sibling pointers. if bad we have a conflict
		 * between the sibling pointers and the child pointers
		 * in the parent block.  blow out the inode if that happens
		 */
		if (bm_cursor->level[level].fsbno != NULLDFSBNO)  {
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
			if (be64_to_cpu(block->bb_u.l.bb_leftsib) != NULLDFSBNO) {
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

		pthread_mutex_lock(&ag_locks[agno]);
		state = get_bmap(agno, agbno);
		switch (state) {
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
_("inode 0x%" PRIu64 "bmap block 0x%" PRIu64 " claimed, state is %d\n"),
				ino, bno, state);
			break;
		case XR_E_MULT:
		case XR_E_INUSE_FS:
			set_bmap(agno, agbno, XR_E_MULT);
			do_warn(
_("inode 0x%" PRIu64 " bmap block 0x%" PRIu64 " claimed, state is %d\n"),
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
_("bad state %d, inode 0x%" PRIu64 " bmap block 0x%" PRIu64 "\n"),
				state, ino, bno);
			break;
		}
		pthread_mutex_unlock(&ag_locks[agno]);
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

	if (level == 0) {
		if (numrecs > mp->m_bmap_dmxr[0] || (isroot == 0 && numrecs <
							mp->m_bmap_dmnr[0])) {
				do_warn(
_("inode 0x%" PRIu64 " bad # of bmap records (%u, min - %u, max - %u)\n"),
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
			err = process_bmbt_reclist(mp, rp, numrecs,
					type, ino, tot, blkmapp,
					&first_key, &last_key,
					whichfork);
			if (err)
				return(1);
			/*
			 * check that key ordering is monotonically increasing.
			 * if the last_key value in the cursor is set to
			 * NULLDFILOFF, then we know this is the first block
			 * on the leaf level and we shouldn't check the
			 * last_key value.
			 */
			if (first_key <= bm_cursor->level[level].last_key &&
					bm_cursor->level[level].last_key !=
					NULLDFILOFF)  {
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

			return(0);
		} else
			return(scan_bmbt_reclist(mp, rp, numrecs,
						type, ino, tot, whichfork));
	}
	if (numrecs > mp->m_bmap_dmxr[1] || (isroot == 0 && numrecs <
							mp->m_bmap_dmnr[1])) {
		do_warn(
_("inode 0x%" PRIu64 " bad # of bmap records (%u, min - %u, max - %u)\n"),
			ino, numrecs, mp->m_bmap_dmnr[1], mp->m_bmap_dmxr[1]);
		return(1);
	}
	pp = XFS_BMBT_PTR_ADDR(mp, block, 1, mp->m_bmap_dmxr[1]);
	pkey = XFS_BMBT_KEY_ADDR(mp, block, 1);

	last_key = NULLDFILOFF;

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

		err = scan_lbtree(be64_to_cpu(pp[i]), level, scanfunc_bmap,
				type, whichfork, ino, tot, nex, blkmapp,
				bm_cursor, 0, check_dups);
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
			bm_cursor->level[level].right_fsbno == NULLDFSBNO &&
			bm_cursor->level[level - 1].right_fsbno != NULLDFSBNO) {
		do_warn(
_("bad fwd (right) sibling pointer (saw %" PRIu64 " should be NULLDFSBNO)\n"
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
scanfunc_bno(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	void			*agcnts)
{
	return scanfunc_allocbt(block, level, bno, agno,
				suspect, isroot, XFS_ABTB_MAGIC, agcnts);
}

static void
scanfunc_cnt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	void			*agcnts)
{
	return scanfunc_allocbt(block, level, bno, agno,
				suspect, isroot, XFS_ABTC_MAGIC, agcnts);
}

static void
scanfunc_allocbt(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	__uint32_t		magic,
	struct aghdr_cnts	*agcnts)
{
	const char 		*name;
	int			i;
	xfs_alloc_ptr_t		*pp;
	xfs_alloc_rec_t		*rp;
	int			hdr_errors = 0;
	int			numrecs;
	int			state;
	xfs_extlen_t		lastcount = 0;
	xfs_agblock_t		lastblock = 0;

	assert(magic == XFS_ABTB_MAGIC || magic == XFS_ABTC_MAGIC);

	name = (magic == XFS_ABTB_MAGIC) ? "bno" : "cnt";

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

			if (magic == XFS_ABTB_MAGIC) {
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
					if (magic == XFS_ABTC_MAGIC) {
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
			scan_sbtree(bno, level, agno, suspect,
				    (magic == XFS_ABTB_MAGIC) ?
				     scanfunc_bno : scanfunc_cnt, 0,
				     (void *)agcnts);
		}
	}
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
	int			off;
	int			state;
	ino_tree_node_t		*ino_rec, *first_rec, *last_rec;

	ino = be32_to_cpu(rp->ir_startino);
	off = XFS_AGINO_TO_OFFSET(mp, ino);
	agbno = XFS_AGINO_TO_AGBNO(mp, ino);
	lino = XFS_AGINO_TO_INO(mp, agno, ino);

	/*
	 * on multi-block block chunks, all chunks start
	 * at the beginning of the block.  with multi-chunk
	 * blocks, all chunks must start on 64-inode boundaries
	 * since each block can hold N complete chunks. if
	 * fs has aligned inodes, all chunks must start
	 * at a fs_ino_alignment*N'th agbno.  skip recs
	 * with badly aligned starting inodes.
	 */
	if (ino == 0 ||
	    (inodes_per_block <= XFS_INODES_PER_CHUNK && off !=  0) ||
	    (inodes_per_block > XFS_INODES_PER_CHUNK &&
	     off % XFS_INODES_PER_CHUNK != 0) ||
	    (fs_aligned_inodes && agbno % fs_ino_alignment != 0))  {
		do_warn(
	_("badly aligned inode rec (starting inode = %" PRIu64 ")\n"),
			lino);
		suspect++;
	}

	/*
	 * verify numeric validity of inode chunk first
	 * before inserting into a tree.  don't have to
	 * worry about the overflow case because the
	 * starting ino number of a chunk can only get
	 * within 255 inodes of max (NULLAGINO).  if it
	 * gets closer, the agino number will be illegal
	 * as the agbno will be too large.
	 */
	if (verify_aginum(mp, agno, ino))  {
		do_warn(
_("bad starting inode # (%" PRIu64 " (0x%x 0x%x)) in ino rec, skipping rec\n"),
			lino, agno, ino);
		return ++suspect;
	}

	if (verify_aginum(mp, agno,
			ino + XFS_INODES_PER_CHUNK - 1))  {
		do_warn(
_("bad ending inode # (%" PRIu64 " (0x%x 0x%zx)) in ino rec, skipping rec\n"),
			lino + XFS_INODES_PER_CHUNK - 1,
			agno,
			ino + XFS_INODES_PER_CHUNK - 1);
		return ++suspect;
	}

	/*
	 * set state of each block containing inodes
	 */
	if (off == 0 && !suspect)  {
		for (j = 0;
		     j < XFS_INODES_PER_CHUNK;
		     j += mp->m_sb.sb_inopblock)  {
			agbno = XFS_AGINO_TO_AGBNO(mp, ino + j);

			state = get_bmap(agno, agbno);
			if (state == XR_E_UNKNOWN)  {
				set_bmap(agno, agbno, XR_E_INO);
			} else if (state == XR_E_INUSE_FS && agno == 0 &&
				   ino + j >= first_prealloc_ino &&
				   ino + j < last_prealloc_ino)  {
				set_bmap(agno, agbno, XR_E_INO);
			} else  {
				do_warn(
_("inode chunk claims used block, inobt block - agno %d, bno %d, inopb %d\n"),
					agno, agbno, mp->m_sb.sb_inopblock);
				/*
				 * XXX - maybe should mark
				 * block a duplicate
				 */
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

	nfree = 0;

	/*
	 * now mark all the inodes as existing and free or used.
	 * if the tree is suspect, put them into the uncertain
	 * inode tree.
	 */
	if (!suspect)  {
		if (XFS_INOBT_IS_FREE_DISK(rp, 0)) {
			nfree++;
			ino_rec = set_inode_free_alloc(mp, agno, ino);
		} else  {
			ino_rec = set_inode_used_alloc(mp, agno, ino);
		}
		for (j = 1; j < XFS_INODES_PER_CHUNK; j++) {
			if (XFS_INOBT_IS_FREE_DISK(rp, j)) {
				nfree++;
				set_inode_free(ino_rec, j);
			} else  {
				set_inode_used(ino_rec, j);
			}
		}
	} else  {
		for (j = 0; j < XFS_INODES_PER_CHUNK; j++) {
			if (XFS_INOBT_IS_FREE_DISK(rp, j)) {
				nfree++;
				add_aginode_uncertain(agno, ino + j, 1);
			} else  {
				add_aginode_uncertain(agno, ino + j, 0);
			}
		}
	}

	if (nfree != be32_to_cpu(rp->ir_freecount)) {
		do_warn(_("ir_freecount/free mismatch, inode "
			"chunk %d/%u, freecount %d nfree %d\n"),
			agno, ino, be32_to_cpu(rp->ir_freecount), nfree);
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
scanfunc_ino(
	struct xfs_btree_block	*block,
	int			level,
	xfs_agblock_t		bno,
	xfs_agnumber_t		agno,
	int			suspect,
	int			isroot,
	void			*priv)
{
	struct aghdr_cnts	*agcnts = priv;
	int			i;
	int			numrecs;
	int			state;
	xfs_inobt_ptr_t		*pp;
	xfs_inobt_rec_t		*rp;
	int			hdr_errors;

	hdr_errors = 0;

	if (be32_to_cpu(block->bb_magic) != XFS_IBT_MAGIC) {
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
			agcnts->agicount += XFS_INODES_PER_CHUNK;
			agcnts->icount += XFS_INODES_PER_CHUNK;
			agcnts->agifreecount += be32_to_cpu(rp[i].ir_freecount);
			agcnts->ifreecount += be32_to_cpu(rp[i].ir_freecount);

			suspect = scan_single_ino_chunk(agno, &rp[i], suspect);
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
					suspect, scanfunc_ino, 0, priv);
	}
}

static void
scan_freelist(
	xfs_agf_t	*agf,
	struct aghdr_cnts *agcnts)
{
	xfs_agfl_t	*agfl;
	xfs_buf_t	*agflbuf;
	xfs_agnumber_t	agno;
	xfs_agblock_t	bno;
	int		count;
	int		i;

	agno = be32_to_cpu(agf->agf_seqno);

	if (XFS_SB_BLOCK(mp) != XFS_AGFL_BLOCK(mp) &&
	    XFS_AGF_BLOCK(mp) != XFS_AGFL_BLOCK(mp) &&
	    XFS_AGI_BLOCK(mp) != XFS_AGFL_BLOCK(mp))
		set_bmap(agno, XFS_AGFL_BLOCK(mp), XR_E_FS_MAP);

	if (be32_to_cpu(agf->agf_flcount) == 0)
		return;

	agflbuf = libxfs_readbuf(mp->m_dev,
				 XFS_AG_DADDR(mp, agno, XFS_AGFL_DADDR(mp)),
				 XFS_FSS_TO_BB(mp, 1), 0);
	if (!agflbuf)  {
		do_abort(_("can't read agfl block for ag %d\n"), agno);
		return;
	}
	agfl = XFS_BUF_TO_AGFL(agflbuf);
	i = be32_to_cpu(agf->agf_flfirst);
	count = 0;
	for (;;) {
		bno = be32_to_cpu(agfl->agfl_bno[i]);
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

	bno = be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNO]);
	if (bno != 0 && verify_agbno(mp, agno, bno)) {
		scan_sbtree(bno, be32_to_cpu(agf->agf_levels[XFS_BTNUM_BNO]),
			    agno, 0, scanfunc_bno, 1, agcnts);
	} else {
		do_warn(_("bad agbno %u for btbno root, agno %d\n"),
			bno, agno);
	}

	bno = be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNT]);
	if (bno != 0 && verify_agbno(mp, agno, bno)) {
		scan_sbtree(bno, be32_to_cpu(agf->agf_levels[XFS_BTNUM_CNT]),
			    agno, 0, scanfunc_cnt, 1, agcnts);
	} else  {
		do_warn(_("bad agbno %u for btbcnt root, agno %d\n"),
			bno, agno);
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

	bno = be32_to_cpu(agi->agi_root);
	if (bno != 0 && verify_agbno(mp, agno, bno)) {
		scan_sbtree(bno, be32_to_cpu(agi->agi_level),
			    agno, 0, scanfunc_ino, 1, agcnts);
	} else {
		do_warn(_("bad agbno %u for inobt root, agno %d\n"),
			be32_to_cpu(agi->agi_root), agno);
	}

	if (be32_to_cpu(agi->agi_count) != agcnts->agicount) {
		do_warn(_("agi_count %u, counted %u in ag %u\n"),
			 be32_to_cpu(agi->agi_count), agcnts->agicount, agno);
	}

	if (be32_to_cpu(agi->agi_freecount) != agcnts->agifreecount) {
		do_warn(_("agi_freecount %u, counted %u in ag %u\n"),
			be32_to_cpu(agi->agi_freecount), agcnts->agifreecount, agno);
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
	xfs_agf_t	*agf;
	xfs_buf_t	*agfbuf;
	int		agf_dirty = 0;
	xfs_agi_t	*agi;
	xfs_buf_t	*agibuf;
	int		agi_dirty = 0;
	xfs_sb_t	*sb;
	xfs_buf_t	*sbbuf;
	int		sb_dirty = 0;
	int		status;

	sbbuf = libxfs_readbuf(mp->m_dev, XFS_AG_DADDR(mp, agno, XFS_SB_DADDR),
				XFS_FSS_TO_BB(mp, 1), 0);
	if (!sbbuf)  {
		do_error(_("can't get root superblock for ag %d\n"), agno);
		return;
	}

	sb = (xfs_sb_t *)calloc(BBSIZE, 1);
	if (!sb) {
		do_error(_("can't allocate memory for superblock\n"));
		libxfs_putbuf(sbbuf);
		return;
	}
	libxfs_sb_from_disk(sb, XFS_BUF_TO_SBP(sbbuf));

	agfbuf = libxfs_readbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), 0);
	if (!agfbuf)  {
		do_error(_("can't read agf block for ag %d\n"), agno);
		libxfs_putbuf(sbbuf);
		free(sb);
		return;
	}
	agf = XFS_BUF_TO_AGF(agfbuf);

	agibuf = libxfs_readbuf(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), 0);
	if (!agibuf)  {
		do_error(_("can't read agi block for ag %d\n"), agno);
		libxfs_putbuf(agfbuf);
		libxfs_putbuf(sbbuf);
		free(sb);
		return;
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
		libxfs_putbuf(agibuf);
		libxfs_putbuf(agfbuf);
		libxfs_putbuf(sbbuf);
		free(sb);

		do_warn(_("bad uncorrected agheader %d, skipping ag...\n"),
			agno);

		return;
	}

	scan_freelist(agf, agcnts);

	validate_agf(agf, agno, agcnts);
	validate_agi(agi, agno, agcnts);

	ASSERT(agi_dirty == 0 || (agi_dirty && !no_modify));

	if (agi_dirty && !no_modify)
		libxfs_writebuf(agibuf, 0);
	else
		libxfs_putbuf(agibuf);

	ASSERT(agf_dirty == 0 || (agf_dirty && !no_modify));

	if (agf_dirty && !no_modify)
		libxfs_writebuf(agfbuf, 0);
	else
		libxfs_putbuf(agfbuf);

	ASSERT(sb_dirty == 0 || (sb_dirty && !no_modify));

	if (sb_dirty && !no_modify) {
		if (agno == 0)
			memcpy(&mp->m_sb, sb, sizeof(xfs_sb_t));
		libxfs_sb_to_disk(XFS_BUF_TO_SBP(sbbuf), sb, XFS_SB_ALL_BITS);
		libxfs_writebuf(sbbuf, 0);
	} else
		libxfs_putbuf(sbbuf);
	free(sb);
	PROG_RPT_INC(prog_rpt_done[agno], 1);

#ifdef XR_INODE_TRACE
	print_inode_list(i);
#endif
	return;
}

#define SCAN_THREADS 32

void
scan_ags(
	struct xfs_mount	*mp,
	int			scan_threads)
{
	struct aghdr_cnts *agcnts;
	__uint64_t	fdblocks = 0;
	__uint64_t	icount = 0;
	__uint64_t	ifreecount = 0;
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
		icount += agcnts[i].icount;
		ifreecount += agcnts[i].ifreecount;
	}

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
}

