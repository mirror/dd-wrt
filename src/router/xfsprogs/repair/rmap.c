// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs.h"
#include "btree.h"
#include "err_protos.h"
#include "libxlog.h"
#include "incore.h"
#include "globals.h"
#include "dinode.h"
#include "slab.h"
#include "rmap.h"
#include "libfrog/bitmap.h"

#undef RMAP_DEBUG

#ifdef RMAP_DEBUG
# define dbg_printf(f, a...)  do {printf(f, ## a); fflush(stdout); } while (0)
#else
# define dbg_printf(f, a...)
#endif

/* per-AG rmap object anchor */
struct xfs_ag_rmap {
	struct xfs_slab	*ar_rmaps;		/* rmap observations, p4 */
	struct xfs_slab	*ar_raw_rmaps;		/* unmerged rmaps */
	int		ar_flcount;		/* agfl entries from leftover */
						/* agbt allocations */
	struct xfs_rmap_irec	ar_last_rmap;	/* last rmap seen */
	struct xfs_slab	*ar_refcount_items;	/* refcount items, p4-5 */
};

static struct xfs_ag_rmap *ag_rmaps;
static bool rmapbt_suspect;
static bool refcbt_suspect;

static inline int rmap_compare(const void *a, const void *b)
{
	return libxfs_rmap_compare(a, b);
}

/*
 * Returns true if we must reconstruct either the reference count or reverse
 * mapping trees.
 */
bool
rmap_needs_work(
	struct xfs_mount	*mp)
{
	return xfs_has_reflink(mp) ||
	       xfs_has_rmapbt(mp);
}

/*
 * Initialize per-AG reverse map data.
 */
void
rmaps_init(
	struct xfs_mount	*mp)
{
	xfs_agnumber_t		i;
	int			error;

	if (!rmap_needs_work(mp))
		return;

	ag_rmaps = calloc(mp->m_sb.sb_agcount, sizeof(struct xfs_ag_rmap));
	if (!ag_rmaps)
		do_error(_("couldn't allocate per-AG reverse map roots\n"));

	for (i = 0; i < mp->m_sb.sb_agcount; i++) {
		error = init_slab(&ag_rmaps[i].ar_rmaps,
				sizeof(struct xfs_rmap_irec));
		if (error)
			do_error(
_("Insufficient memory while allocating reverse mapping slabs."));
		error = init_slab(&ag_rmaps[i].ar_raw_rmaps,
				  sizeof(struct xfs_rmap_irec));
		if (error)
			do_error(
_("Insufficient memory while allocating raw metadata reverse mapping slabs."));
		ag_rmaps[i].ar_last_rmap.rm_owner = XFS_RMAP_OWN_UNKNOWN;
		error = init_slab(&ag_rmaps[i].ar_refcount_items,
				  sizeof(struct xfs_refcount_irec));
		if (error)
			do_error(
_("Insufficient memory while allocating refcount item slabs."));
	}
}

/*
 * Free the per-AG reverse-mapping data.
 */
void
rmaps_free(
	struct xfs_mount	*mp)
{
	xfs_agnumber_t		i;

	if (!rmap_needs_work(mp))
		return;

	for (i = 0; i < mp->m_sb.sb_agcount; i++) {
		free_slab(&ag_rmaps[i].ar_rmaps);
		free_slab(&ag_rmaps[i].ar_raw_rmaps);
		free_slab(&ag_rmaps[i].ar_refcount_items);
	}
	free(ag_rmaps);
	ag_rmaps = NULL;
}

/*
 * Decide if two reverse-mapping records can be merged.
 */
bool
rmaps_are_mergeable(
	struct xfs_rmap_irec	*r1,
	struct xfs_rmap_irec	*r2)
{
	if (r1->rm_owner != r2->rm_owner)
		return false;
	if (r1->rm_startblock + r1->rm_blockcount != r2->rm_startblock)
		return false;
	if ((unsigned long long)r1->rm_blockcount + r2->rm_blockcount >
	    XFS_RMAP_LEN_MAX)
		return false;
	if (XFS_RMAP_NON_INODE_OWNER(r2->rm_owner))
		return true;
	/* must be an inode owner below here */
	if (r1->rm_flags != r2->rm_flags)
		return false;
	if (r1->rm_flags & XFS_RMAP_BMBT_BLOCK)
		return true;
	return r1->rm_offset + r1->rm_blockcount == r2->rm_offset;
}

/*
 * Add an observation about a block mapping in an inode's data or attribute
 * fork for later btree reconstruction.
 */
int
rmap_add_rec(
	struct xfs_mount	*mp,
	xfs_ino_t		ino,
	int			whichfork,
	struct xfs_bmbt_irec	*irec)
{
	struct xfs_rmap_irec	rmap;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	struct xfs_rmap_irec	*last_rmap;
	int			error = 0;

	if (!rmap_needs_work(mp))
		return 0;

	agno = XFS_FSB_TO_AGNO(mp, irec->br_startblock);
	agbno = XFS_FSB_TO_AGBNO(mp, irec->br_startblock);
	ASSERT(agno != NULLAGNUMBER);
	ASSERT(agno < mp->m_sb.sb_agcount);
	ASSERT(agbno + irec->br_blockcount <= mp->m_sb.sb_agblocks);
	ASSERT(ino != NULLFSINO);
	ASSERT(whichfork == XFS_DATA_FORK || whichfork == XFS_ATTR_FORK);

	rmap.rm_owner = ino;
	rmap.rm_offset = irec->br_startoff;
	rmap.rm_flags = 0;
	if (whichfork == XFS_ATTR_FORK)
		rmap.rm_flags |= XFS_RMAP_ATTR_FORK;
	rmap.rm_startblock = agbno;
	rmap.rm_blockcount = irec->br_blockcount;
	if (irec->br_state == XFS_EXT_UNWRITTEN)
		rmap.rm_flags |= XFS_RMAP_UNWRITTEN;
	last_rmap = &ag_rmaps[agno].ar_last_rmap;
	if (last_rmap->rm_owner == XFS_RMAP_OWN_UNKNOWN)
		*last_rmap = rmap;
	else if (rmaps_are_mergeable(last_rmap, &rmap))
		last_rmap->rm_blockcount += rmap.rm_blockcount;
	else {
		error = slab_add(ag_rmaps[agno].ar_rmaps, last_rmap);
		if (error)
			return error;
		*last_rmap = rmap;
	}

	return error;
}

/* Finish collecting inode data/attr fork rmaps. */
int
rmap_finish_collecting_fork_recs(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno)
{
	if (!rmap_needs_work(mp) ||
	    ag_rmaps[agno].ar_last_rmap.rm_owner == XFS_RMAP_OWN_UNKNOWN)
		return 0;
	return slab_add(ag_rmaps[agno].ar_rmaps, &ag_rmaps[agno].ar_last_rmap);
}

/* add a raw rmap; these will be merged later */
static int
__rmap_add_raw_rec(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	xfs_extlen_t		len,
	uint64_t		owner,
	bool			is_attr,
	bool			is_bmbt)
{
	struct xfs_rmap_irec	rmap;

	ASSERT(len != 0);
	rmap.rm_owner = owner;
	rmap.rm_offset = 0;
	rmap.rm_flags = 0;
	if (is_attr)
		rmap.rm_flags |= XFS_RMAP_ATTR_FORK;
	if (is_bmbt)
		rmap.rm_flags |= XFS_RMAP_BMBT_BLOCK;
	rmap.rm_startblock = agbno;
	rmap.rm_blockcount = len;
	return slab_add(ag_rmaps[agno].ar_raw_rmaps, &rmap);
}

/*
 * Add a reverse mapping for an inode fork's block mapping btree block.
 */
int
rmap_add_bmbt_rec(
	struct xfs_mount	*mp,
	xfs_ino_t		ino,
	int			whichfork,
	xfs_fsblock_t		fsbno)
{
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;

	if (!rmap_needs_work(mp))
		return 0;

	agno = XFS_FSB_TO_AGNO(mp, fsbno);
	agbno = XFS_FSB_TO_AGBNO(mp, fsbno);
	ASSERT(agno != NULLAGNUMBER);
	ASSERT(agno < mp->m_sb.sb_agcount);
	ASSERT(agbno + 1 <= mp->m_sb.sb_agblocks);

	return __rmap_add_raw_rec(mp, agno, agbno, 1, ino,
			whichfork == XFS_ATTR_FORK, true);
}

/*
 * Add a reverse mapping for a per-AG fixed metadata extent.
 */
int
rmap_add_ag_rec(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	xfs_extlen_t		len,
	uint64_t		owner)
{
	if (!rmap_needs_work(mp))
		return 0;

	ASSERT(agno != NULLAGNUMBER);
	ASSERT(agno < mp->m_sb.sb_agcount);
	ASSERT(agbno + len <= mp->m_sb.sb_agblocks);

	return __rmap_add_raw_rec(mp, agno, agbno, len, owner, false, false);
}

/*
 * Merge adjacent raw rmaps and add them to the main rmap list.
 */
int
rmap_fold_raw_recs(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno)
{
	struct xfs_slab_cursor	*cur = NULL;
	struct xfs_rmap_irec	*prev, *rec;
	size_t			old_sz;
	int			error = 0;

	old_sz = slab_count(ag_rmaps[agno].ar_rmaps);
	if (slab_count(ag_rmaps[agno].ar_raw_rmaps) == 0)
		goto no_raw;
	qsort_slab(ag_rmaps[agno].ar_raw_rmaps, rmap_compare);
	error = init_slab_cursor(ag_rmaps[agno].ar_raw_rmaps, rmap_compare,
			&cur);
	if (error)
		goto err;

	prev = pop_slab_cursor(cur);
	rec = pop_slab_cursor(cur);
	while (prev && rec) {
		if (rmaps_are_mergeable(prev, rec)) {
			prev->rm_blockcount += rec->rm_blockcount;
			rec = pop_slab_cursor(cur);
			continue;
		}
		error = slab_add(ag_rmaps[agno].ar_rmaps, prev);
		if (error)
			goto err;
		prev = rec;
		rec = pop_slab_cursor(cur);
	}
	if (prev) {
		error = slab_add(ag_rmaps[agno].ar_rmaps, prev);
		if (error)
			goto err;
	}
	free_slab(&ag_rmaps[agno].ar_raw_rmaps);
	error = init_slab(&ag_rmaps[agno].ar_raw_rmaps,
			sizeof(struct xfs_rmap_irec));
	if (error)
		do_error(
_("Insufficient memory while allocating raw metadata reverse mapping slabs."));
no_raw:
	if (old_sz)
		qsort_slab(ag_rmaps[agno].ar_rmaps, rmap_compare);
err:
	free_slab_cursor(&cur);
	return error;
}

static int
find_first_zero_bit(
	uint64_t	mask)
{
	int		n;
	int		b = 0;

	for (n = 0; n < sizeof(mask) * NBBY && (mask & 1); n++, mask >>= 1)
		b++;

	return b;
}

static int
popcnt(
	uint64_t	mask)
{
	int		n;
	int		b = 0;

	if (mask == 0)
		return 0;

	for (n = 0; n < sizeof(mask) * NBBY; n++, mask >>= 1)
		if (mask & 1)
			b++;

	return b;
}

/*
 * Add an allocation group's fixed metadata to the rmap list.  This includes
 * sb/agi/agf/agfl headers, inode chunks, and the log.
 */
int
rmap_add_fixed_ag_rec(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno)
{
	xfs_fsblock_t		fsbno;
	xfs_agblock_t		agbno;
	ino_tree_node_t		*ino_rec;
	xfs_agino_t		agino;
	int			error;
	int			startidx;
	int			nr;

	if (!rmap_needs_work(mp))
		return 0;

	/* sb/agi/agf/agfl headers */
	error = rmap_add_ag_rec(mp, agno, 0, XFS_BNO_BLOCK(mp),
			XFS_RMAP_OWN_FS);
	if (error)
		goto out;

	/* inodes */
	ino_rec = findfirst_inode_rec(agno);
	for (; ino_rec != NULL; ino_rec = next_ino_rec(ino_rec)) {
		if (xfs_has_sparseinodes(mp)) {
			startidx = find_first_zero_bit(ino_rec->ir_sparse);
			nr = XFS_INODES_PER_CHUNK - popcnt(ino_rec->ir_sparse);
		} else {
			startidx = 0;
			nr = XFS_INODES_PER_CHUNK;
		}
		nr /= mp->m_sb.sb_inopblock;
		if (nr == 0)
			nr = 1;
		agino = ino_rec->ino_startnum + startidx;
		agbno = XFS_AGINO_TO_AGBNO(mp, agino);
		if (XFS_AGINO_TO_OFFSET(mp, agino) == 0) {
			error = rmap_add_ag_rec(mp, agno, agbno, nr,
					XFS_RMAP_OWN_INODES);
			if (error)
				goto out;
		}
	}

	/* log */
	fsbno = mp->m_sb.sb_logstart;
	if (fsbno && XFS_FSB_TO_AGNO(mp, fsbno) == agno) {
		agbno = XFS_FSB_TO_AGBNO(mp, mp->m_sb.sb_logstart);
		error = rmap_add_ag_rec(mp, agno, agbno, mp->m_sb.sb_logblocks,
				XFS_RMAP_OWN_LOG);
		if (error)
			goto out;
	}
out:
	return error;
}

/*
 * Copy the per-AG btree reverse-mapping data into the rmapbt.
 *
 * At rmapbt reconstruction time, the rmapbt will be populated _only_ with
 * rmaps for file extents, inode chunks, AG headers, and bmbt blocks.  While
 * building the AG btrees we can record all the blocks allocated for each
 * btree, but we cannot resolve the conflict between the fact that one has to
 * finish allocating the space for the rmapbt before building the bnobt and the
 * fact that allocating blocks for the bnobt requires adding rmapbt entries.
 * Therefore we record in-core the rmaps for each btree and here use the
 * libxfs rmap functions to finish building the rmap btree.
 *
 * During AGF/AGFL reconstruction in phase 5, rmaps for the AG btrees are
 * recorded in memory.  The rmapbt has not been set up yet, so we need to be
 * able to "expand" the AGFL without updating the rmapbt.  After we've written
 * out the new AGF header the new rmapbt is available, so this function reads
 * each AGFL to generate rmap entries.  These entries are merged with the AG
 * btree rmap entries, and then we use libxfs' rmap functions to add them to
 * the rmapbt, after which it is fully regenerated.
 */
int
rmap_store_ag_btree_rec(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno)
{
	struct xfs_slab_cursor	*rm_cur;
	struct xfs_rmap_irec	*rm_rec = NULL;
	struct xfs_buf		*agbp = NULL;
	struct xfs_buf		*agflbp = NULL;
	struct xfs_trans	*tp;
	__be32			*agfl_bno, *b;
	struct xfs_ag_rmap	*ag_rmap = &ag_rmaps[agno];
	struct bitmap		*own_ag_bitmap = NULL;
	int			error = 0;

	if (!xfs_has_rmapbt(mp))
		return 0;

	/* Release the ar_rmaps; they were put into the rmapbt during p5. */
	free_slab(&ag_rmap->ar_rmaps);
	error = init_slab(&ag_rmap->ar_rmaps, sizeof(struct xfs_rmap_irec));
	if (error)
		goto err;

	/* Add the AGFL blocks to the rmap list */
	error = -libxfs_trans_read_buf(
			mp, NULL, mp->m_ddev_targp,
			XFS_AG_DADDR(mp, agno, XFS_AGFL_DADDR(mp)),
			XFS_FSS_TO_BB(mp, 1), 0, &agflbp, &xfs_agfl_buf_ops);
	if (error)
		goto err;

	/*
	 * Sometimes, the blocks at the beginning of the AGFL are there
	 * because we overestimated how many blocks we needed to rebuild
	 * the freespace btrees.  ar_flcount records the number of
	 * blocks in this situation.  Since those blocks already have an
	 * rmap, we only need to add rmap records for AGFL blocks past
	 * that point in the AGFL because those blocks are a result of a
	 * no-rmap no-shrink freelist fixup that we did earlier.
	 *
	 * However, some blocks end up on the AGFL because the free space
	 * btrees shed blocks as a result of allocating space to fix the
	 * freelist.  We already created in-core rmap records for the free
	 * space btree blocks, so we must be careful not to create those
	 * records again.  Create a bitmap of already-recorded OWN_AG rmaps.
	 */
	error = init_slab_cursor(ag_rmap->ar_raw_rmaps, rmap_compare, &rm_cur);
	if (error)
		goto err;
	error = -bitmap_alloc(&own_ag_bitmap);
	if (error)
		goto err_slab;
	while ((rm_rec = pop_slab_cursor(rm_cur)) != NULL) {
		if (rm_rec->rm_owner != XFS_RMAP_OWN_AG)
			continue;
		error = -bitmap_set(own_ag_bitmap, rm_rec->rm_startblock,
					rm_rec->rm_blockcount);
		if (error) {
			/*
			 * If this range is already set, then the incore rmap
			 * records for the AG free space btrees overlap and
			 * we're toast because that is not allowed.
			 */
			if (error == EEXIST)
				error = EFSCORRUPTED;
			goto err_slab;
		}
	}
	free_slab_cursor(&rm_cur);

	/* Create rmaps for any AGFL blocks that aren't already rmapped. */
	agfl_bno = xfs_buf_to_agfl_bno(agflbp);
	b = agfl_bno + ag_rmap->ar_flcount;
	while (*b != cpu_to_be32(NULLAGBLOCK) &&
	       b - agfl_bno < libxfs_agfl_size(mp)) {
		xfs_agblock_t	agbno;

		agbno = be32_to_cpu(*b);
		if (!bitmap_test(own_ag_bitmap, agbno, 1)) {
			error = rmap_add_ag_rec(mp, agno, agbno, 1,
					XFS_RMAP_OWN_AG);
			if (error)
				goto err;
		}
		b++;
	}
	libxfs_buf_relse(agflbp);
	agflbp = NULL;
	bitmap_free(&own_ag_bitmap);

	/* Merge all the raw rmaps into the main list */
	error = rmap_fold_raw_recs(mp, agno);
	if (error)
		goto err;

	/* Create cursors to refcount structures */
	error = init_slab_cursor(ag_rmap->ar_rmaps, rmap_compare, &rm_cur);
	if (error)
		goto err;

	/* Insert rmaps into the btree one at a time */
	rm_rec = pop_slab_cursor(rm_cur);
	while (rm_rec) {
		struct xfs_owner_info	oinfo = {};
		struct xfs_perag	*pag;

		error = -libxfs_trans_alloc_rollable(mp, 16, &tp);
		if (error)
			goto err_slab;

		pag = libxfs_perag_get(mp, agno);
		error = -libxfs_alloc_read_agf(pag, tp, 0, &agbp);
		if (error) {
			libxfs_perag_put(pag);
			goto err_trans;
		}

		ASSERT(XFS_RMAP_NON_INODE_OWNER(rm_rec->rm_owner));
		oinfo.oi_owner = rm_rec->rm_owner;
		error = -libxfs_rmap_alloc(tp, agbp, pag, rm_rec->rm_startblock,
				rm_rec->rm_blockcount, &oinfo);
		libxfs_perag_put(pag);
		if (error)
			goto err_trans;

		error = -libxfs_trans_commit(tp);
		if (error)
			goto err_slab;

		fix_freelist(mp, agno, false);

		rm_rec = pop_slab_cursor(rm_cur);
	}

	free_slab_cursor(&rm_cur);
	return 0;

err_trans:
	libxfs_trans_cancel(tp);
err_slab:
	free_slab_cursor(&rm_cur);
err:
	if (agflbp)
		libxfs_buf_relse(agflbp);
	if (own_ag_bitmap)
		bitmap_free(&own_ag_bitmap);
	return error;
}

#ifdef RMAP_DEBUG
static void
rmap_dump(
	const char		*msg,
	xfs_agnumber_t		agno,
	struct xfs_rmap_irec	*rmap)
{
	printf("%s: %p agno=%u pblk=%llu own=%lld lblk=%llu len=%u flags=0x%x\n",
		msg, rmap,
		(unsigned int)agno,
		(unsigned long long)rmap->rm_startblock,
		(unsigned long long)rmap->rm_owner,
		(unsigned long long)rmap->rm_offset,
		(unsigned int)rmap->rm_blockcount,
		(unsigned int)rmap->rm_flags);
}
#else
# define rmap_dump(m, a, r)
#endif

/*
 * Rebuilding the Reference Count & Reverse Mapping Btrees
 *
 * The reference count (refcnt) and reverse mapping (rmap) btrees are
 * rebuilt during phase 5, like all other AG btrees.  Therefore, reverse
 * mappings must be processed into reference counts at the end of phase
 * 4, and the rmaps must be recorded during phase 4.  There is a need to
 * access the rmaps in physical block order, but no particular need for
 * random access, so the slab.c code provides a big logical array
 * (consisting of smaller slabs) and some inorder iterator functions.
 *
 * Once we've recorded all the reverse mappings, we're ready to
 * translate the rmaps into refcount entries.  Imagine the rmap entries
 * as rectangles representing extents of physical blocks, and that the
 * rectangles can be laid down to allow them to overlap each other; then
 * we know that we must emit a refcnt btree entry wherever the amount of
 * overlap changes, i.e. the emission stimulus is level-triggered:
 *
 *                 -    ---
 *       --      ----- ----   ---        ------
 * --   ----     ----------- ----     ---------
 * -------------------------------- -----------
 * ^ ^  ^^ ^^    ^ ^^ ^^^  ^^^^  ^ ^^ ^  ^     ^
 * 2 1  23 21    3 43 234  2123  1 01 2  3     0
 *
 * For our purposes, a rmap is a tuple (startblock, len, fileoff, owner).
 *
 * Note that in the actual refcnt btree we don't store the refcount < 2
 * cases because the bnobt tells us which blocks are free; single-use
 * blocks aren't recorded in the bnobt or the refcntbt.  If the rmapbt
 * supports storing multiple entries covering a given block we could
 * theoretically dispense with the refcntbt and simply count rmaps, but
 * that's inefficient in the (hot) write path, so we'll take the cost of
 * the extra tree to save time.  Also there's no guarantee that rmap
 * will be enabled.
 *
 * Given an array of rmaps sorted by physical block number, a starting
 * physical block (sp), a bag to hold rmaps that cover sp, and the next
 * physical block where the level changes (np), we can reconstruct the
 * refcount btree as follows:
 *
 * While there are still unprocessed rmaps in the array,
 *  - Set sp to the physical block (pblk) of the next unprocessed rmap.
 *  - Add to the bag all rmaps in the array where startblock == sp.
 *  - Set np to the physical block where the bag size will change.  This
 *    is the minimum of (the pblk of the next unprocessed rmap) and
 *    (startblock + len of each rmap in the bag).
 *  - Record the bag size as old_bag_size.
 *
 *  - While the bag isn't empty,
 *     - Remove from the bag all rmaps where startblock + len == np.
 *     - Add to the bag all rmaps in the array where startblock == np.
 *     - If the bag size isn't old_bag_size, store the refcount entry
 *       (sp, np - sp, bag_size) in the refcnt btree.
 *     - If the bag is empty, break out of the inner loop.
 *     - Set old_bag_size to the bag size
 *     - Set sp = np.
 *     - Set np to the physical block where the bag size will change.
 *       This is the minimum of (the pblk of the next unprocessed rmap)
 *       and (startblock + len of each rmap in the bag).
 *
 * An implementation detail is that because this processing happens
 * during phase 4, the refcount entries are stored in an array so that
 * phase 5 can load them into the refcount btree.  The rmaps can be
 * loaded directly into the rmap btree during phase 5 as well.
 */

/*
 * Mark all inodes in the reverse-mapping observation stack as requiring the
 * reflink inode flag, if the stack depth is greater than 1.
 */
static void
mark_inode_rl(
	struct xfs_mount		*mp,
	struct xfs_bag		*rmaps)
{
	xfs_agnumber_t		iagno;
	struct xfs_rmap_irec	*rmap;
	struct ino_tree_node	*irec;
	int			off;
	size_t			idx;
	xfs_agino_t		ino;

	if (bag_count(rmaps) < 2)
		return;

	/* Reflink flag accounting */
	foreach_bag_ptr(rmaps, idx, rmap) {
		ASSERT(!XFS_RMAP_NON_INODE_OWNER(rmap->rm_owner));
		iagno = XFS_INO_TO_AGNO(mp, rmap->rm_owner);
		ino = XFS_INO_TO_AGINO(mp, rmap->rm_owner);
		pthread_mutex_lock(&ag_locks[iagno].lock);
		irec = find_inode_rec(mp, iagno, ino);
		off = get_inode_offset(mp, rmap->rm_owner, irec);
		/* lock here because we might go outside this ag */
		set_inode_is_rl(irec, off);
		pthread_mutex_unlock(&ag_locks[iagno].lock);
	}
}

/*
 * Emit a refcount object for refcntbt reconstruction during phase 5.
 */
#define REFCOUNT_CLAMP(nr)	((nr) > MAXREFCOUNT ? MAXREFCOUNT : (nr))
static void
refcount_emit(
	struct xfs_mount		*mp,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno,
	xfs_extlen_t		len,
	size_t			nr_rmaps)
{
	struct xfs_refcount_irec	rlrec;
	int			error;
	struct xfs_slab		*rlslab;

	rlslab = ag_rmaps[agno].ar_refcount_items;
	ASSERT(nr_rmaps > 0);

	dbg_printf("REFL: agno=%u pblk=%u, len=%u -> refcount=%zu\n",
		agno, agbno, len, nr_rmaps);
	rlrec.rc_startblock = agbno;
	rlrec.rc_blockcount = len;
	rlrec.rc_refcount = REFCOUNT_CLAMP(nr_rmaps);
	rlrec.rc_domain = XFS_REFC_DOMAIN_SHARED;

	error = slab_add(rlslab, &rlrec);
	if (error)
		do_error(
_("Insufficient memory while recreating refcount tree."));
}
#undef REFCOUNT_CLAMP

/*
 * Transform a pile of physical block mapping observations into refcount data
 * for eventual rebuilding of the btrees.
 */
#define RMAP_END(r)	((r)->rm_startblock + (r)->rm_blockcount)
int
compute_refcounts(
	struct xfs_mount		*mp,
	xfs_agnumber_t		agno)
{
	struct xfs_bag		*stack_top = NULL;
	struct xfs_slab		*rmaps;
	struct xfs_slab_cursor	*rmaps_cur;
	struct xfs_rmap_irec	*array_cur;
	struct xfs_rmap_irec	*rmap;
	xfs_agblock_t		sbno;	/* first bno of this rmap set */
	xfs_agblock_t		cbno;	/* first bno of this refcount set */
	xfs_agblock_t		nbno;	/* next bno where rmap set changes */
	size_t			n, idx;
	size_t			old_stack_nr;
	int			error;

	if (!xfs_has_reflink(mp))
		return 0;

	rmaps = ag_rmaps[agno].ar_rmaps;

	error = init_slab_cursor(rmaps, rmap_compare, &rmaps_cur);
	if (error)
		return error;

	error = init_bag(&stack_top);
	if (error)
		goto err;

	/* While there are rmaps to be processed... */
	n = 0;
	while (n < slab_count(rmaps)) {
		array_cur = peek_slab_cursor(rmaps_cur);
		sbno = cbno = array_cur->rm_startblock;
		/* Push all rmaps with pblk == sbno onto the stack */
		for (;
		     array_cur && array_cur->rm_startblock == sbno;
		     array_cur = peek_slab_cursor(rmaps_cur)) {
			advance_slab_cursor(rmaps_cur); n++;
			rmap_dump("push0", agno, array_cur);
			error = bag_add(stack_top, array_cur);
			if (error)
				goto err;
		}
		mark_inode_rl(mp, stack_top);

		/* Set nbno to the bno of the next refcount change */
		if (n < slab_count(rmaps) && array_cur)
			nbno = array_cur->rm_startblock;
		else
			nbno = NULLAGBLOCK;
		foreach_bag_ptr(stack_top, idx, rmap) {
			nbno = min(nbno, RMAP_END(rmap));
		}

		/* Emit reverse mappings, if needed */
		ASSERT(nbno > sbno);
		old_stack_nr = bag_count(stack_top);

		/* While stack isn't empty... */
		while (bag_count(stack_top)) {
			/* Pop all rmaps that end at nbno */
			foreach_bag_ptr_reverse(stack_top, idx, rmap) {
				if (RMAP_END(rmap) != nbno)
					continue;
				rmap_dump("pop", agno, rmap);
				error = bag_remove(stack_top, idx);
				if (error)
					goto err;
			}

			/* Push array items that start at nbno */
			for (;
			     array_cur && array_cur->rm_startblock == nbno;
			     array_cur = peek_slab_cursor(rmaps_cur)) {
				advance_slab_cursor(rmaps_cur); n++;
				rmap_dump("push1", agno, array_cur);
				error = bag_add(stack_top, array_cur);
				if (error)
					goto err;
			}
			mark_inode_rl(mp, stack_top);

			/* Emit refcount if necessary */
			ASSERT(nbno > cbno);
			if (bag_count(stack_top) != old_stack_nr) {
				if (old_stack_nr > 1) {
					refcount_emit(mp, agno, cbno,
						      nbno - cbno,
						      old_stack_nr);
				}
				cbno = nbno;
			}

			/* Stack empty, go find the next rmap */
			if (bag_count(stack_top) == 0)
				break;
			old_stack_nr = bag_count(stack_top);
			sbno = nbno;

			/* Set nbno to the bno of the next refcount change */
			if (n < slab_count(rmaps))
				nbno = array_cur->rm_startblock;
			else
				nbno = NULLAGBLOCK;
			foreach_bag_ptr(stack_top, idx, rmap) {
				nbno = min(nbno, RMAP_END(rmap));
			}

			/* Emit reverse mappings, if needed */
			ASSERT(nbno > sbno);
		}
	}
err:
	free_bag(&stack_top);
	free_slab_cursor(&rmaps_cur);

	return error;
}
#undef RMAP_END

/*
 * Return the number of rmap objects for an AG.
 */
size_t
rmap_record_count(
	struct xfs_mount		*mp,
	xfs_agnumber_t		agno)
{
	return slab_count(ag_rmaps[agno].ar_rmaps);
}

/*
 * Return a slab cursor that will return rmap objects in order.
 */
int
rmap_init_cursor(
	xfs_agnumber_t		agno,
	struct xfs_slab_cursor	**cur)
{
	return init_slab_cursor(ag_rmaps[agno].ar_rmaps, rmap_compare, cur);
}

/*
 * Disable the refcount btree check.
 */
void
rmap_avoid_check(void)
{
	rmapbt_suspect = true;
}

/* Look for an rmap in the rmapbt that matches a given rmap. */
static int
rmap_lookup(
	struct xfs_btree_cur	*bt_cur,
	struct xfs_rmap_irec	*rm_rec,
	struct xfs_rmap_irec	*tmp,
	int			*have)
{
	/* Use the regular btree retrieval routine. */
	return -libxfs_rmap_lookup_le(bt_cur, rm_rec->rm_startblock,
				rm_rec->rm_owner, rm_rec->rm_offset,
				rm_rec->rm_flags, tmp, have);
}

/* Look for an rmap in the rmapbt that matches a given rmap. */
static int
rmap_lookup_overlapped(
	struct xfs_btree_cur	*bt_cur,
	struct xfs_rmap_irec	*rm_rec,
	struct xfs_rmap_irec	*tmp,
	int			*have)
{
	/* Have to use our fancy version for overlapped */
	return -libxfs_rmap_lookup_le_range(bt_cur, rm_rec->rm_startblock,
				rm_rec->rm_owner, rm_rec->rm_offset,
				rm_rec->rm_flags, tmp, have);
}

/* Does the btree rmap cover the observed rmap? */
#define NEXTP(x)	((x)->rm_startblock + (x)->rm_blockcount)
#define NEXTL(x)	((x)->rm_offset + (x)->rm_blockcount)
static bool
rmap_is_good(
	struct xfs_rmap_irec	*observed,
	struct xfs_rmap_irec	*btree)
{
	/* Can't have mismatches in the flags or the owner. */
	if (btree->rm_flags != observed->rm_flags ||
	    btree->rm_owner != observed->rm_owner)
		return false;

	/*
	 * Btree record can't physically start after the observed
	 * record, nor can it end before the observed record.
	 */
	if (btree->rm_startblock > observed->rm_startblock ||
	    NEXTP(btree) < NEXTP(observed))
		return false;

	/* If this is metadata or bmbt, we're done. */
	if (XFS_RMAP_NON_INODE_OWNER(observed->rm_owner) ||
	    (observed->rm_flags & XFS_RMAP_BMBT_BLOCK))
		return true;
	/*
	 * Btree record can't logically start after the observed
	 * record, nor can it end before the observed record.
	 */
	if (btree->rm_offset > observed->rm_offset ||
	    NEXTL(btree) < NEXTL(observed))
		return false;

	return true;
}
#undef NEXTP
#undef NEXTL

/*
 * Compare the observed reverse mappings against what's in the ag btree.
 */
void
rmaps_verify_btree(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno)
{
	struct xfs_rmap_irec	tmp;
	struct xfs_slab_cursor	*rm_cur;
	struct xfs_btree_cur	*bt_cur = NULL;
	struct xfs_buf		*agbp = NULL;
	struct xfs_rmap_irec	*rm_rec;
	struct xfs_perag	*pag = NULL;
	int			have;
	int			error;

	if (!xfs_has_rmapbt(mp))
		return;
	if (rmapbt_suspect) {
		if (no_modify && agno == 0)
			do_warn(_("would rebuild corrupt rmap btrees.\n"));
		return;
	}

	/* Create cursors to refcount structures */
	error = rmap_init_cursor(agno, &rm_cur);
	if (error) {
		do_warn(_("Not enough memory to check reverse mappings.\n"));
		return;
	}

	pag = libxfs_perag_get(mp, agno);
	error = -libxfs_alloc_read_agf(pag, NULL, 0, &agbp);
	if (error) {
		do_warn(_("Could not read AGF %u to check rmap btree.\n"),
				agno);
		goto err_pag;
	}

	/* Leave the per-ag data "uninitialized" since we rewrite it later */
	clear_bit(XFS_AGSTATE_AGF_INIT, &pag->pag_opstate);

	bt_cur = libxfs_rmapbt_init_cursor(mp, NULL, agbp, pag);
	if (!bt_cur) {
		do_warn(_("Not enough memory to check reverse mappings.\n"));
		goto err_agf;
	}

	rm_rec = pop_slab_cursor(rm_cur);
	while (rm_rec) {
		error = rmap_lookup(bt_cur, rm_rec, &tmp, &have);
		if (error) {
			do_warn(
_("Could not read reverse-mapping record for (%u/%u).\n"),
					agno, rm_rec->rm_startblock);
			goto err_cur;
		}

		/*
		 * Using the range query is expensive, so only do it if
		 * the regular lookup doesn't find anything or if it doesn't
		 * match the observed rmap.
		 */
		if (xfs_has_reflink(bt_cur->bc_mp) &&
				(!have || !rmap_is_good(rm_rec, &tmp))) {
			error = rmap_lookup_overlapped(bt_cur, rm_rec,
					&tmp, &have);
			if (error) {
				do_warn(
_("Could not read reverse-mapping record for (%u/%u).\n"),
						agno, rm_rec->rm_startblock);
				goto err_cur;
			}
		}
		if (!have) {
			do_warn(
_("Missing reverse-mapping record for (%u/%u) %slen %u owner %"PRId64" \
%s%soff %"PRIu64"\n"),
				agno, rm_rec->rm_startblock,
				(rm_rec->rm_flags & XFS_RMAP_UNWRITTEN) ?
					_("unwritten ") : "",
				rm_rec->rm_blockcount,
				rm_rec->rm_owner,
				(rm_rec->rm_flags & XFS_RMAP_ATTR_FORK) ?
					_("attr ") : "",
				(rm_rec->rm_flags & XFS_RMAP_BMBT_BLOCK) ?
					_("bmbt ") : "",
				rm_rec->rm_offset);
			goto next_loop;
		}

		/* Compare each refcount observation against the btree's */
		if (!rmap_is_good(rm_rec, &tmp)) {
			do_warn(
_("Incorrect reverse-mapping: saw (%u/%u) %slen %u owner %"PRId64" %s%soff \
%"PRIu64"; should be (%u/%u) %slen %u owner %"PRId64" %s%soff %"PRIu64"\n"),
				agno, tmp.rm_startblock,
				(tmp.rm_flags & XFS_RMAP_UNWRITTEN) ?
					_("unwritten ") : "",
				tmp.rm_blockcount,
				tmp.rm_owner,
				(tmp.rm_flags & XFS_RMAP_ATTR_FORK) ?
					_("attr ") : "",
				(tmp.rm_flags & XFS_RMAP_BMBT_BLOCK) ?
					_("bmbt ") : "",
				tmp.rm_offset,
				agno, rm_rec->rm_startblock,
				(rm_rec->rm_flags & XFS_RMAP_UNWRITTEN) ?
					_("unwritten ") : "",
				rm_rec->rm_blockcount,
				rm_rec->rm_owner,
				(rm_rec->rm_flags & XFS_RMAP_ATTR_FORK) ?
					_("attr ") : "",
				(rm_rec->rm_flags & XFS_RMAP_BMBT_BLOCK) ?
					_("bmbt ") : "",
				rm_rec->rm_offset);
			goto next_loop;
		}
next_loop:
		rm_rec = pop_slab_cursor(rm_cur);
	}

err_cur:
	libxfs_btree_del_cursor(bt_cur, XFS_BTREE_NOERROR);
err_agf:
	libxfs_buf_relse(agbp);
err_pag:
	libxfs_perag_put(pag);
	free_slab_cursor(&rm_cur);
}

/*
 * Compare the key fields of two rmap records -- positive if key1 > key2,
 * negative if key1 < key2, and zero if equal.
 */
int64_t
rmap_diffkeys(
	struct xfs_rmap_irec	*kp1,
	struct xfs_rmap_irec	*kp2)
{
	__u64			oa;
	__u64			ob;
	int64_t			d;
	struct xfs_rmap_irec	tmp;

	tmp = *kp1;
	tmp.rm_flags &= ~XFS_RMAP_REC_FLAGS;
	oa = libxfs_rmap_irec_offset_pack(&tmp);
	tmp = *kp2;
	tmp.rm_flags &= ~XFS_RMAP_REC_FLAGS;
	ob = libxfs_rmap_irec_offset_pack(&tmp);

	d = (int64_t)kp1->rm_startblock - kp2->rm_startblock;
	if (d)
		return d;

	if (kp1->rm_owner > kp2->rm_owner)
		return 1;
	else if (kp2->rm_owner > kp1->rm_owner)
		return -1;

	if (oa > ob)
		return 1;
	else if (ob > oa)
		return -1;
	return 0;
}

/* Compute the high key of an rmap record. */
void
rmap_high_key_from_rec(
	struct xfs_rmap_irec	*rec,
	struct xfs_rmap_irec	*key)
{
	int			adj;

	adj = rec->rm_blockcount - 1;

	key->rm_startblock = rec->rm_startblock + adj;
	key->rm_owner = rec->rm_owner;
	key->rm_offset = rec->rm_offset;
	key->rm_flags = rec->rm_flags & XFS_RMAP_KEY_FLAGS;
	if (XFS_RMAP_NON_INODE_OWNER(rec->rm_owner) ||
	    (rec->rm_flags & XFS_RMAP_BMBT_BLOCK))
		return;
	key->rm_offset += adj;
}

/*
 * Record that an inode had the reflink flag set when repair started.  The
 * inode reflink flag will be adjusted as necessary.
 */
void
record_inode_reflink_flag(
	struct xfs_mount	*mp,
	struct xfs_dinode	*dino,
	xfs_agnumber_t		agno,
	xfs_agino_t		ino,
	xfs_ino_t		lino)
{
	struct ino_tree_node	*irec;
	int			off;

	ASSERT(XFS_AGINO_TO_INO(mp, agno, ino) == be64_to_cpu(dino->di_ino));
	if (!(be64_to_cpu(dino->di_flags2) & XFS_DIFLAG2_REFLINK))
		return;
	irec = find_inode_rec(mp, agno, ino);
	off = get_inode_offset(mp, lino, irec);
	ASSERT(!inode_was_rl(irec, off));
	set_inode_was_rl(irec, off);
	dbg_printf("set was_rl lino=%llu was=0x%llx\n",
		(unsigned long long)lino, (unsigned long long)irec->ino_was_rl);
}

/*
 * Inform the user that we're clearing the reflink flag on an inode that
 * doesn't actually share any blocks.  This is an optimization (the kernel
 * skips refcount checks for non-reflink files) and not a corruption repair,
 * so we don't need to log every time we clear a flag unless verbose mode is
 * enabled.
 */
static void
warn_clearing_reflink(
	xfs_ino_t		ino)
{
	static bool		warned = false;
	static pthread_mutex_t	lock = PTHREAD_MUTEX_INITIALIZER;

	if (verbose) {
		do_warn(_("clearing reflink flag on inode %"PRIu64"\n"), ino);
		return;
	}

	if (warned)
		return;

	pthread_mutex_lock(&lock);
	if (!warned) {
		do_warn(_("clearing reflink flag on inodes when possible\n"));
		warned = true;
	}
	pthread_mutex_unlock(&lock);
}

/*
 * Fix an inode's reflink flag.
 */
static int
fix_inode_reflink_flag(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	xfs_agino_t		agino,
	bool			set)
{
	struct xfs_dinode	*dino;
	struct xfs_buf		*buf;

	if (set)
		do_warn(
_("setting reflink flag on inode %"PRIu64"\n"),
			XFS_AGINO_TO_INO(mp, agno, agino));
	else if (!no_modify) /* && !set */
		warn_clearing_reflink(XFS_AGINO_TO_INO(mp, agno, agino));
	if (no_modify)
		return 0;

	buf = get_agino_buf(mp, agno, agino, &dino);
	if (!buf)
		return 1;
	ASSERT(XFS_AGINO_TO_INO(mp, agno, agino) == be64_to_cpu(dino->di_ino));
	if (set)
		dino->di_flags2 |= cpu_to_be64(XFS_DIFLAG2_REFLINK);
	else
		dino->di_flags2 &= cpu_to_be64(~XFS_DIFLAG2_REFLINK);
	libxfs_dinode_calc_crc(mp, dino);
	libxfs_buf_mark_dirty(buf);
	libxfs_buf_relse(buf);

	return 0;
}

/*
 * Fix discrepancies between the state of the inode reflink flag and our
 * observations as to whether or not the inode really needs it.
 */
int
fix_inode_reflink_flags(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno)
{
	struct ino_tree_node	*irec;
	int			bit;
	uint64_t		was;
	uint64_t		is;
	uint64_t		diff;
	uint64_t		mask;
	int			error = 0;
	xfs_agino_t		agino;

	/*
	 * Update the reflink flag for any inode where there's a discrepancy
	 * between the inode flag and whether or not we found any reflinked
	 * extents.
	 */
	for (irec = findfirst_inode_rec(agno);
	     irec != NULL;
	     irec = next_ino_rec(irec)) {
		ASSERT((irec->ino_was_rl & irec->ir_free) == 0);
		ASSERT((irec->ino_is_rl & irec->ir_free) == 0);
		was = irec->ino_was_rl;
		is = irec->ino_is_rl;
		if (was == is)
			continue;
		diff = was ^ is;
		dbg_printf("mismatch ino=%llu was=0x%lx is=0x%lx dif=0x%lx\n",
			(unsigned long long)XFS_AGINO_TO_INO(mp, agno,
						irec->ino_startnum),
			was, is, diff);

		for (bit = 0, mask = 1; bit < 64; bit++, mask <<= 1) {
			agino = bit + irec->ino_startnum;
			if (!(diff & mask))
				continue;
			else if (was & mask)
				error = fix_inode_reflink_flag(mp, agno, agino,
						false);
			else if (is & mask)
				error = fix_inode_reflink_flag(mp, agno, agino,
						true);
			else
				ASSERT(0);
			if (error)
				do_error(
_("Unable to fix reflink flag on inode %"PRIu64".\n"),
					XFS_AGINO_TO_INO(mp, agno, agino));
		}
	}

	return error;
}

/*
 * Return the number of refcount objects for an AG.
 */
size_t
refcount_record_count(
	struct xfs_mount		*mp,
	xfs_agnumber_t		agno)
{
	return slab_count(ag_rmaps[agno].ar_refcount_items);
}

/*
 * Return a slab cursor that will return refcount objects in order.
 */
int
init_refcount_cursor(
	xfs_agnumber_t		agno,
	struct xfs_slab_cursor	**cur)
{
	return init_slab_cursor(ag_rmaps[agno].ar_refcount_items, NULL, cur);
}

/*
 * Disable the refcount btree check.
 */
void
refcount_avoid_check(void)
{
	refcbt_suspect = true;
}

/*
 * Compare the observed reference counts against what's in the ag btree.
 */
void
check_refcounts(
	struct xfs_mount		*mp,
	xfs_agnumber_t			agno)
{
	struct xfs_refcount_irec	tmp;
	struct xfs_slab_cursor		*rl_cur;
	struct xfs_btree_cur		*bt_cur = NULL;
	struct xfs_buf			*agbp = NULL;
	struct xfs_perag		*pag = NULL;
	struct xfs_refcount_irec	*rl_rec;
	int				have;
	int				i;
	int				error;

	if (!xfs_has_reflink(mp))
		return;
	if (refcbt_suspect) {
		if (no_modify && agno == 0)
			do_warn(_("would rebuild corrupt refcount btrees.\n"));
		return;
	}

	/* Create cursors to refcount structures */
	error = init_refcount_cursor(agno, &rl_cur);
	if (error) {
		do_warn(_("Not enough memory to check refcount data.\n"));
		return;
	}

	pag = libxfs_perag_get(mp, agno);
	error = -libxfs_alloc_read_agf(pag, NULL, 0, &agbp);
	if (error) {
		do_warn(_("Could not read AGF %u to check refcount btree.\n"),
				agno);
		goto err_pag;
	}

	/* Leave the per-ag data "uninitialized" since we rewrite it later */
	clear_bit(XFS_AGSTATE_AGF_INIT, &pag->pag_opstate);

	bt_cur = libxfs_refcountbt_init_cursor(mp, NULL, agbp, pag);
	if (!bt_cur) {
		do_warn(_("Not enough memory to check refcount data.\n"));
		goto err_agf;
	}

	rl_rec = pop_slab_cursor(rl_cur);
	while (rl_rec) {
		/* Look for a refcount record in the btree */
		error = -libxfs_refcount_lookup_le(bt_cur,
				XFS_REFC_DOMAIN_SHARED, rl_rec->rc_startblock,
				&have);
		if (error) {
			do_warn(
_("Could not read reference count record for (%u/%u).\n"),
					agno, rl_rec->rc_startblock);
			goto err_cur;
		}
		if (!have) {
			do_warn(
_("Missing reference count record for (%u/%u) len %u count %u\n"),
				agno, rl_rec->rc_startblock,
				rl_rec->rc_blockcount, rl_rec->rc_refcount);
			goto next_loop;
		}

		error = -libxfs_refcount_get_rec(bt_cur, &tmp, &i);
		if (error) {
			do_warn(
_("Could not read reference count record for (%u/%u).\n"),
					agno, rl_rec->rc_startblock);
			goto err_cur;
		}
		if (!i) {
			do_warn(
_("Missing reference count record for (%u/%u) len %u count %u\n"),
				agno, rl_rec->rc_startblock,
				rl_rec->rc_blockcount, rl_rec->rc_refcount);
			goto next_loop;
		}

		/* Compare each refcount observation against the btree's */
		if (tmp.rc_domain != rl_rec->rc_domain ||
		    tmp.rc_startblock != rl_rec->rc_startblock ||
		    tmp.rc_blockcount != rl_rec->rc_blockcount ||
		    tmp.rc_refcount != rl_rec->rc_refcount) {
			unsigned int	start;

			start = xfs_refcount_encode_startblock(
					tmp.rc_startblock, tmp.rc_domain);

			do_warn(
_("Incorrect reference count: saw (%u/%u) len %u nlinks %u; should be (%u/%u) len %u nlinks %u\n"),
				agno, start, tmp.rc_blockcount,
				tmp.rc_refcount, agno, rl_rec->rc_startblock,
				rl_rec->rc_blockcount, rl_rec->rc_refcount);
		}
next_loop:
		rl_rec = pop_slab_cursor(rl_cur);
	}

err_cur:
	libxfs_btree_del_cursor(bt_cur, error);
err_agf:
	libxfs_buf_relse(agbp);
err_pag:
	libxfs_perag_put(pag);
	free_slab_cursor(&rl_cur);
}

/*
 * Regenerate the AGFL so that we don't run out of it while rebuilding the
 * rmap btree.  If skip_rmapbt is true, don't update the rmapbt (most probably
 * because we're updating the rmapbt).
 */
void
fix_freelist(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	bool			skip_rmapbt)
{
	xfs_alloc_arg_t		args;
	xfs_trans_t		*tp;
	int			flags;
	int			error;

	memset(&args, 0, sizeof(args));
	args.mp = mp;
	args.agno = agno;
	args.alignment = 1;
	args.pag = libxfs_perag_get(mp, agno);
	error = -libxfs_trans_alloc_rollable(mp, 0, &tp);
	if (error)
		do_error(_("failed to fix AGFL on AG %d, error %d\n"),
				agno, error);
	args.tp = tp;

	/*
	 * Prior to rmapbt, all we had to do to fix the freelist is "expand"
	 * the fresh AGFL header from empty to full.  That hasn't changed.  For
	 * rmapbt, however, things change a bit.
	 *
	 * When we're stuffing the rmapbt with the AG btree rmaps the tree can
	 * expand, so we need to keep the AGFL well-stocked for the expansion.
	 * However, this expansion can cause the bnobt/cntbt to shrink, which
	 * can make the AGFL eligible for shrinking.  Shrinking involves
	 * freeing rmapbt entries, but since we haven't finished loading the
	 * rmapbt with the btree rmaps it's possible for the remove operation
	 * to fail.  The AGFL block is large enough at this point to absorb any
	 * blocks freed from the bnobt/cntbt, so we can disable shrinking.
	 *
	 * During the initial AGFL regeneration during AGF generation in phase5
	 * we must also disable rmapbt modifications because the AGF that
	 * libxfs reads does not yet point to the new rmapbt.  These initial
	 * AGFL entries are added just prior to adding the AG btree block rmaps
	 * to the rmapbt.  It's ok to pass NOSHRINK here too, since the AGFL is
	 * empty and cannot shrink.
	 */
	flags = XFS_ALLOC_FLAG_NOSHRINK;
	if (skip_rmapbt)
		flags |= XFS_ALLOC_FLAG_NORMAP;
	error = -libxfs_alloc_fix_freelist(&args, flags);
	libxfs_perag_put(args.pag);
	if (error) {
		do_error(_("failed to fix AGFL on AG %d, error %d\n"),
				agno, error);
	}
	error = -libxfs_trans_commit(tp);
	if (error)
		do_error(_("%s: commit failed, error %d\n"), __func__, error);
}

/*
 * Remember how many AGFL entries came from excess AG btree allocations and
 * therefore already have rmap entries.
 */
void
rmap_store_agflcount(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	int			count)
{
	if (!rmap_needs_work(mp))
		return;

	ag_rmaps[agno].ar_flcount = count;
}

/* Estimate the size of the ondisk rmapbt from the incore data. */
xfs_extlen_t
estimate_rmapbt_blocks(
	struct xfs_perag	*pag)
{
	struct xfs_mount	*mp = pag->pag_mount;
	struct xfs_ag_rmap	*x;
	unsigned long long	nr_recs = 0;

	if (!rmap_needs_work(mp) || !xfs_has_rmapbt(mp))
		return 0;

	/*
	 * Overestimate the amount of space needed by pretending that every
	 * record in the incore slab will become rmapbt records.
	 */
	x = &ag_rmaps[pag->pag_agno];
	if (x->ar_rmaps)
		nr_recs += slab_count(x->ar_rmaps);
	if (x->ar_raw_rmaps)
		nr_recs += slab_count(x->ar_raw_rmaps);

	return libxfs_rmapbt_calc_size(mp, nr_recs);
}

/* Estimate the size of the ondisk refcountbt from the incore data. */
xfs_extlen_t
estimate_refcountbt_blocks(
	struct xfs_perag	*pag)
{
	struct xfs_mount	*mp = pag->pag_mount;
	struct xfs_ag_rmap	*x;

	if (!rmap_needs_work(mp) || !xfs_has_reflink(mp))
		return 0;

	x = &ag_rmaps[pag->pag_agno];
	if (!x->ar_refcount_items)
		return 0;

	return libxfs_refcountbt_calc_size(mp,
			slab_count(x->ar_refcount_items));
}
