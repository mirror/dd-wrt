// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include <libxfs.h>
#include "err_protos.h"
#include "libfrog/bitmap.h"
#include "slab.h"
#include "rmap.h"
#include "incore.h"
#include "bulkload.h"
#include "agbtree.h"

/* Initialize a btree rebuild context. */
static void
init_rebuild(
	struct repair_ctx		*sc,
	const struct xfs_owner_info	*oinfo,
	xfs_agblock_t			est_agfreeblocks,
	struct bt_rebuild		*btr)
{
	memset(btr, 0, sizeof(struct bt_rebuild));

	bulkload_init_ag(&btr->newbt, sc, oinfo);
	bulkload_estimate_ag_slack(sc, &btr->bload, est_agfreeblocks);
}

/*
 * Update this free space record to reflect the blocks we stole from the
 * beginning of the record.
 */
static void
consume_freespace(
	xfs_agnumber_t		agno,
	struct extent_tree_node	*ext_ptr,
	uint32_t		len)
{
	struct extent_tree_node	*bno_ext_ptr;
	xfs_agblock_t		new_start = ext_ptr->ex_startblock + len;
	xfs_extlen_t		new_len = ext_ptr->ex_blockcount - len;

	/* Delete the used-up extent from both extent trees. */
#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "releasing extent: %u [%u %u]\n", agno,
			ext_ptr->ex_startblock, ext_ptr->ex_blockcount);
#endif
	bno_ext_ptr = find_bno_extent(agno, ext_ptr->ex_startblock);
	ASSERT(bno_ext_ptr != NULL);
	get_bno_extent(agno, bno_ext_ptr);
	release_extent_tree_node(bno_ext_ptr);

	ext_ptr = get_bcnt_extent(agno, ext_ptr->ex_startblock,
			ext_ptr->ex_blockcount);
	release_extent_tree_node(ext_ptr);

	/*
	 * If we only used part of this last extent, then we must reinsert the
	 * extent to maintain proper sorting order.
	 */
	if (new_len > 0) {
		add_bno_extent(agno, new_start, new_len);
		add_bcnt_extent(agno, new_start, new_len);
	}
}

/*
 * Reserve blocks for the new per-AG structures.  Returns true if all blocks
 * were allocated, and false if we ran out of space.
 */
static bool
reserve_agblocks(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr,
	uint32_t		nr_blocks)
{
	struct extent_tree_node	*ext_ptr;
	uint32_t		blocks_allocated = 0;
	uint32_t		len;
	int			error;

	while (blocks_allocated < nr_blocks)  {
		xfs_fsblock_t	fsbno;

		/*
		 * Grab the smallest extent and use it up, then get the
		 * next smallest.  This mimics the init_*_cursor code.
		 */
		ext_ptr = findfirst_bcnt_extent(agno);
		if (!ext_ptr)
			break;

		/* Use up the extent we've got. */
		len = min(ext_ptr->ex_blockcount, nr_blocks - blocks_allocated);
		fsbno = XFS_AGB_TO_FSB(mp, agno, ext_ptr->ex_startblock);
		error = bulkload_add_blocks(&btr->newbt, fsbno, len);
		if (error)
			do_error(_("could not set up btree reservation: %s\n"),
				strerror(-error));

		error = rmap_add_ag_rec(mp, agno, ext_ptr->ex_startblock, len,
				btr->newbt.oinfo.oi_owner);
		if (error)
			do_error(_("could not set up btree rmaps: %s\n"),
				strerror(-error));

		consume_freespace(agno, ext_ptr, len);
		blocks_allocated += len;
	}
#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "blocks_allocated = %d\n",
		blocks_allocated);
#endif
	return blocks_allocated == nr_blocks;
}

static inline void
reserve_btblocks(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr,
	uint32_t		nr_blocks)
{
	if (!reserve_agblocks(mp, agno, btr, nr_blocks))
		do_error(
	_("error - not enough free space in filesystem, AG %u\n"),
				agno);
}

/* Feed one of the new btree blocks to the bulk loader. */
static int
rebuild_claim_block(
	struct xfs_btree_cur	*cur,
	union xfs_btree_ptr	*ptr,
	void			*priv)
{
	struct bt_rebuild	*btr = priv;

	return bulkload_claim_block(cur, &btr->newbt, ptr);
}

/*
 * Scoop up leftovers from a rebuild cursor for later freeing, then free the
 * rebuild context.
 */
void
finish_rebuild(
	struct xfs_mount	*mp,
	struct bt_rebuild	*btr,
	struct bitmap		*lost_blocks)
{
	struct bulkload_resv	*resv, *n;
	int			error;

	for_each_bulkload_reservation(&btr->newbt, resv, n) {
		if (resv->used == resv->len)
			continue;

		error = bitmap_set(lost_blocks, resv->fsbno + resv->used,
				   resv->len - resv->used);
		if (error)
			do_error(
_("Insufficient memory saving lost blocks, err=%d.\n"), error);
		resv->used = resv->len;
	}

	bulkload_destroy(&btr->newbt, 0);
}

/*
 * Free Space Btrees
 *
 * We need to leave some free records in the tree for the corner case of
 * setting up the AGFL. This may require allocation of blocks, and as
 * such can require insertion of new records into the tree (e.g. moving
 * a record in the by-count tree when a long extent is shortened). If we
 * pack the records into the leaves with no slack space, this requires a
 * leaf split to occur and a block to be allocated from the free list.
 * If we don't have any blocks on the free list (because we are setting
 * it up!), then we fail, and the filesystem will fail with the same
 * failure at runtime. Hence leave a couple of records slack space in
 * each block to allow immediate modification of the tree without
 * requiring splits to be done.
 */

/*
 * Return the next free space extent tree record from the previous value we
 * saw.
 */
static inline struct extent_tree_node *
get_bno_rec(
	struct xfs_btree_cur	*cur,
	struct extent_tree_node	*prev_value)
{
	xfs_agnumber_t		agno = cur->bc_ag.pag->pag_agno;

	if (cur->bc_btnum == XFS_BTNUM_BNO) {
		if (!prev_value)
			return findfirst_bno_extent(agno);
		return findnext_bno_extent(prev_value);
	}

	/* cnt btree */
	if (!prev_value)
		return findfirst_bcnt_extent(agno);
	return findnext_bcnt_extent(agno, prev_value);
}

/* Grab one bnobt record and put it in the btree cursor. */
static int
get_bnobt_record(
	struct xfs_btree_cur		*cur,
	void				*priv)
{
	struct bt_rebuild		*btr = priv;
	struct xfs_alloc_rec_incore	*arec = &cur->bc_rec.a;

	btr->bno_rec = get_bno_rec(cur, btr->bno_rec);
	arec->ar_startblock = btr->bno_rec->ex_startblock;
	arec->ar_blockcount = btr->bno_rec->ex_blockcount;
	btr->freeblks += btr->bno_rec->ex_blockcount;
	return 0;
}

void
init_freespace_cursors(
	struct repair_ctx	*sc,
	struct xfs_perag	*pag,
	unsigned int		est_agfreeblocks,
	unsigned int		*nr_extents,
	int			*extra_blocks,
	struct bt_rebuild	*btr_bno,
	struct bt_rebuild	*btr_cnt)
{
	xfs_agnumber_t		agno = pag->pag_agno;
	unsigned int		agfl_goal;
	int			error;

	agfl_goal = libxfs_alloc_min_freelist(sc->mp, NULL);

	init_rebuild(sc, &XFS_RMAP_OINFO_AG, est_agfreeblocks, btr_bno);
	init_rebuild(sc, &XFS_RMAP_OINFO_AG, est_agfreeblocks, btr_cnt);

	btr_bno->cur = libxfs_allocbt_stage_cursor(sc->mp,
			&btr_bno->newbt.afake, pag, XFS_BTNUM_BNO);
	btr_cnt->cur = libxfs_allocbt_stage_cursor(sc->mp,
			&btr_cnt->newbt.afake, pag, XFS_BTNUM_CNT);

	btr_bno->bload.get_record = get_bnobt_record;
	btr_bno->bload.claim_block = rebuild_claim_block;

	btr_cnt->bload.get_record = get_bnobt_record;
	btr_cnt->bload.claim_block = rebuild_claim_block;

	/*
	 * Now we need to allocate blocks for the free space btrees using the
	 * free space records we're about to put in them.  Every record we use
	 * can change the shape of the free space trees, so we recompute the
	 * btree shape until we stop needing /more/ blocks.  If we have any
	 * left over we'll stash them in the AGFL when we're done.
	 */
	do {
		unsigned int	num_freeblocks;
		int		delta_bno, delta_cnt;
		int		agfl_wanted;

		/* Compute how many bnobt blocks we'll need. */
		error = -libxfs_btree_bload_compute_geometry(btr_bno->cur,
				&btr_bno->bload, *nr_extents);
		if (error)
			do_error(
_("Unable to compute free space by block btree geometry, error %d.\n"), -error);

		/* Compute how many cntbt blocks we'll need. */
		error = -libxfs_btree_bload_compute_geometry(btr_cnt->cur,
				&btr_cnt->bload, *nr_extents);
		if (error)
			do_error(
_("Unable to compute free space by length btree geometry, error %d.\n"), -error);

		/*
		 * Compute the deficit between the number of blocks reserved
		 * and the number of blocks we think we need for the btree.
		 */
		delta_bno = (int)btr_bno->newbt.nr_reserved -
				 btr_bno->bload.nr_blocks;
		delta_cnt = (int)btr_cnt->newbt.nr_reserved -
				 btr_cnt->bload.nr_blocks;

		/* We don't need any more blocks, so we're done. */
		if (delta_bno >= 0 && delta_cnt >= 0 &&
		    delta_bno + delta_cnt >= agfl_goal) {
			*extra_blocks = delta_bno + delta_cnt;
			break;
		}

		/* Allocate however many more blocks we need this time. */
		if (delta_bno < 0) {
			reserve_btblocks(sc->mp, agno, btr_bno, -delta_bno);
			delta_bno = 0;
		}
		if (delta_cnt < 0) {
			reserve_btblocks(sc->mp, agno, btr_cnt, -delta_cnt);
			delta_cnt = 0;
		}

		/*
		 * Try to fill the bnobt cursor with extra blocks to populate
		 * the AGFL.  If we don't get all the blocks we want, stop
		 * trying to fill the AGFL because the AG is totally out of
		 * space.
		 */
		agfl_wanted = agfl_goal - (delta_bno + delta_cnt);
		if (agfl_wanted > 0 &&
		    !reserve_agblocks(sc->mp, agno, btr_bno, agfl_wanted))
			agfl_goal = 0;

		/* Ok, now how many free space records do we have? */
		*nr_extents = count_bno_extents_blocks(agno, &num_freeblocks);
	} while (1);
}

/* Rebuild the free space btrees. */
void
build_freespace_btrees(
	struct repair_ctx	*sc,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr_bno,
	struct bt_rebuild	*btr_cnt)
{
	int			error;

	/* Add all observed bnobt records. */
	error = -libxfs_btree_bload(btr_bno->cur, &btr_bno->bload, btr_bno);
	if (error)
		do_error(
_("Error %d while creating bnobt btree for AG %u.\n"), error, agno);

	/* Add all observed cntbt records. */
	error = -libxfs_btree_bload(btr_cnt->cur, &btr_cnt->bload, btr_cnt);
	if (error)
		do_error(
_("Error %d while creating cntbt btree for AG %u.\n"), error, agno);

	/* Since we're not writing the AGF yet, no need to commit the cursor */
	libxfs_btree_del_cursor(btr_bno->cur, 0);
	libxfs_btree_del_cursor(btr_cnt->cur, 0);
}

/* Inode Btrees */

static inline struct ino_tree_node *
get_ino_rec(
	struct xfs_btree_cur	*cur,
	struct ino_tree_node	*prev_value)
{
	xfs_agnumber_t		agno = cur->bc_ag.pag->pag_agno;

	if (cur->bc_btnum == XFS_BTNUM_INO) {
		if (!prev_value)
			return findfirst_inode_rec(agno);
		return next_ino_rec(prev_value);
	}

	/* finobt */
	if (!prev_value)
		return findfirst_free_inode_rec(agno);
	return next_free_ino_rec(prev_value);
}

/* Grab one inobt record. */
static int
get_inobt_record(
	struct xfs_btree_cur		*cur,
	void				*priv)
{
	struct bt_rebuild		*btr = priv;
	struct xfs_inobt_rec_incore	*irec = &cur->bc_rec.i;
	struct ino_tree_node		*ino_rec;
	int				inocnt = 0;
	int				finocnt = 0;
	int				k;

	btr->ino_rec = ino_rec = get_ino_rec(cur, btr->ino_rec);

	/* Transform the incore record into an on-disk record. */
	irec->ir_startino = ino_rec->ino_startnum;
	irec->ir_free = ino_rec->ir_free;

	for (k = 0; k < sizeof(xfs_inofree_t) * NBBY; k++)  {
		ASSERT(is_inode_confirmed(ino_rec, k));

		if (is_inode_sparse(ino_rec, k))
			continue;
		if (is_inode_free(ino_rec, k))
			finocnt++;
		inocnt++;
	}

	irec->ir_count = inocnt;
	irec->ir_freecount = finocnt;

	if (xfs_has_sparseinodes(cur->bc_mp)) {
		uint64_t		sparse;
		int			spmask;
		uint16_t		holemask;

		/*
		 * Convert the 64-bit in-core sparse inode state to the
		 * 16-bit on-disk holemask.
		 */
		holemask = 0;
		spmask = (1 << XFS_INODES_PER_HOLEMASK_BIT) - 1;
		sparse = ino_rec->ir_sparse;
		for (k = 0; k < XFS_INOBT_HOLEMASK_BITS; k++) {
			if (sparse & spmask) {
				ASSERT((sparse & spmask) == spmask);
				holemask |= (1 << k);
			} else
				ASSERT((sparse & spmask) == 0);
			sparse >>= XFS_INODES_PER_HOLEMASK_BIT;
		}

		irec->ir_holemask = holemask;
	} else {
		irec->ir_holemask = 0;
	}

	if (btr->first_agino == NULLAGINO)
		btr->first_agino = ino_rec->ino_startnum;
	btr->freecount += finocnt;
	btr->count += inocnt;
	return 0;
}

/* Initialize both inode btree cursors as needed. */
void
init_ino_cursors(
	struct repair_ctx	*sc,
	struct xfs_perag	*pag,
	unsigned int		est_agfreeblocks,
	uint64_t		*num_inos,
	uint64_t		*num_free_inos,
	struct bt_rebuild	*btr_ino,
	struct bt_rebuild	*btr_fino)
{
	struct ino_tree_node	*ino_rec;
	xfs_agnumber_t		agno = pag->pag_agno;
	unsigned int		ino_recs = 0;
	unsigned int		fino_recs = 0;
	bool			finobt;
	int			error;

	finobt = xfs_has_finobt(sc->mp);
	init_rebuild(sc, &XFS_RMAP_OINFO_INOBT, est_agfreeblocks, btr_ino);

	/* Compute inode statistics. */
	*num_free_inos = 0;
	*num_inos = 0;
	for (ino_rec = findfirst_inode_rec(agno);
	     ino_rec != NULL;
	     ino_rec = next_ino_rec(ino_rec))  {
		unsigned int	rec_ninos = 0;
		unsigned int	rec_nfinos = 0;
		int		i;

		for (i = 0; i < XFS_INODES_PER_CHUNK; i++)  {
			ASSERT(is_inode_confirmed(ino_rec, i));
			/*
			 * sparse inodes are not factored into superblock (free)
			 * inode counts
			 */
			if (is_inode_sparse(ino_rec, i))
				continue;
			if (is_inode_free(ino_rec, i))
				rec_nfinos++;
			rec_ninos++;
		}

		*num_free_inos += rec_nfinos;
		*num_inos += rec_ninos;
		ino_recs++;

		/* finobt only considers records with free inodes */
		if (rec_nfinos)
			fino_recs++;
	}

	btr_ino->cur = libxfs_inobt_stage_cursor(pag, &btr_ino->newbt.afake,
			XFS_BTNUM_INO);

	btr_ino->bload.get_record = get_inobt_record;
	btr_ino->bload.claim_block = rebuild_claim_block;
	btr_ino->first_agino = NULLAGINO;

	/* Compute how many inobt blocks we'll need. */
	error = -libxfs_btree_bload_compute_geometry(btr_ino->cur,
			&btr_ino->bload, ino_recs);
	if (error)
		do_error(
_("Unable to compute inode btree geometry, error %d.\n"), error);

	reserve_btblocks(sc->mp, agno, btr_ino, btr_ino->bload.nr_blocks);

	if (!finobt)
		return;

	init_rebuild(sc, &XFS_RMAP_OINFO_INOBT, est_agfreeblocks, btr_fino);
	btr_fino->cur = libxfs_inobt_stage_cursor(pag,
			&btr_fino->newbt.afake, XFS_BTNUM_FINO);

	btr_fino->bload.get_record = get_inobt_record;
	btr_fino->bload.claim_block = rebuild_claim_block;
	btr_fino->first_agino = NULLAGINO;

	/* Compute how many finobt blocks we'll need. */
	error = -libxfs_btree_bload_compute_geometry(btr_fino->cur,
			&btr_fino->bload, fino_recs);
	if (error)
		do_error(
_("Unable to compute free inode btree geometry, error %d.\n"), error);

	reserve_btblocks(sc->mp, agno, btr_fino, btr_fino->bload.nr_blocks);
}

/* Rebuild the inode btrees. */
void
build_inode_btrees(
	struct repair_ctx	*sc,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr_ino,
	struct bt_rebuild	*btr_fino)
{
	int			error;

	/* Add all observed inobt records. */
	error = -libxfs_btree_bload(btr_ino->cur, &btr_ino->bload, btr_ino);
	if (error)
		do_error(
_("Error %d while creating inobt btree for AG %u.\n"), error, agno);

	/* Since we're not writing the AGI yet, no need to commit the cursor */
	libxfs_btree_del_cursor(btr_ino->cur, 0);

	if (!xfs_has_finobt(sc->mp))
		return;

	/* Add all observed finobt records. */
	error = -libxfs_btree_bload(btr_fino->cur, &btr_fino->bload, btr_fino);
	if (error)
		do_error(
_("Error %d while creating finobt btree for AG %u.\n"), error, agno);

	/* Since we're not writing the AGI yet, no need to commit the cursor */
	libxfs_btree_del_cursor(btr_fino->cur, 0);
}

/* rebuild the rmap tree */

/* Grab one rmap record. */
static int
get_rmapbt_record(
	struct xfs_btree_cur		*cur,
	void				*priv)
{
	struct xfs_rmap_irec		*rec;
	struct bt_rebuild		*btr = priv;

	rec = pop_slab_cursor(btr->slab_cursor);
	memcpy(&cur->bc_rec.r, rec, sizeof(struct xfs_rmap_irec));
	return 0;
}

/* Set up the rmap rebuild parameters. */
void
init_rmapbt_cursor(
	struct repair_ctx	*sc,
	struct xfs_perag	*pag,
	unsigned int		est_agfreeblocks,
	struct bt_rebuild	*btr)
{
	xfs_agnumber_t		agno = pag->pag_agno;
	int			error;

	if (!xfs_has_rmapbt(sc->mp))
		return;

	init_rebuild(sc, &XFS_RMAP_OINFO_AG, est_agfreeblocks, btr);
	btr->cur = libxfs_rmapbt_stage_cursor(sc->mp, &btr->newbt.afake, pag);

	btr->bload.get_record = get_rmapbt_record;
	btr->bload.claim_block = rebuild_claim_block;

	/* Compute how many blocks we'll need. */
	error = -libxfs_btree_bload_compute_geometry(btr->cur, &btr->bload,
			rmap_record_count(sc->mp, agno));
	if (error)
		do_error(
_("Unable to compute rmap btree geometry, error %d.\n"), error);

	reserve_btblocks(sc->mp, agno, btr, btr->bload.nr_blocks);
}

/* Rebuild a rmap btree. */
void
build_rmap_tree(
	struct repair_ctx	*sc,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr)
{
	int			error;

	error = rmap_init_cursor(agno, &btr->slab_cursor);
	if (error)
		do_error(
_("Insufficient memory to construct rmap cursor.\n"));

	/* Add all observed rmap records. */
	error = -libxfs_btree_bload(btr->cur, &btr->bload, btr);
	if (error)
		do_error(
_("Error %d while creating rmap btree for AG %u.\n"), error, agno);

	/* Since we're not writing the AGF yet, no need to commit the cursor */
	libxfs_btree_del_cursor(btr->cur, 0);
	free_slab_cursor(&btr->slab_cursor);
}

/* rebuild the refcount tree */

/* Grab one refcount record. */
static int
get_refcountbt_record(
	struct xfs_btree_cur		*cur,
	void				*priv)
{
	struct xfs_refcount_irec	*rec;
	struct bt_rebuild		*btr = priv;

	rec = pop_slab_cursor(btr->slab_cursor);
	memcpy(&cur->bc_rec.rc, rec, sizeof(struct xfs_refcount_irec));
	return 0;
}

/* Set up the refcount rebuild parameters. */
void
init_refc_cursor(
	struct repair_ctx	*sc,
	struct xfs_perag	*pag,
	unsigned int		est_agfreeblocks,
	struct bt_rebuild	*btr)
{
	xfs_agnumber_t		agno = pag->pag_agno;
	int			error;

	if (!xfs_has_reflink(sc->mp))
		return;

	init_rebuild(sc, &XFS_RMAP_OINFO_REFC, est_agfreeblocks, btr);
	btr->cur = libxfs_refcountbt_stage_cursor(sc->mp, &btr->newbt.afake,
			pag);

	btr->bload.get_record = get_refcountbt_record;
	btr->bload.claim_block = rebuild_claim_block;

	/* Compute how many blocks we'll need. */
	error = -libxfs_btree_bload_compute_geometry(btr->cur, &btr->bload,
			refcount_record_count(sc->mp, agno));
	if (error)
		do_error(
_("Unable to compute refcount btree geometry, error %d.\n"), error);

	reserve_btblocks(sc->mp, agno, btr, btr->bload.nr_blocks);
}

/* Rebuild a refcount btree. */
void
build_refcount_tree(
	struct repair_ctx	*sc,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr)
{
	int			error;

	error = init_refcount_cursor(agno, &btr->slab_cursor);
	if (error)
		do_error(
_("Insufficient memory to construct refcount cursor.\n"));

	/* Add all observed refcount records. */
	error = -libxfs_btree_bload(btr->cur, &btr->bload, btr);
	if (error)
		do_error(
_("Error %d while creating refcount btree for AG %u.\n"), error, agno);

	/* Since we're not writing the AGF yet, no need to commit the cursor */
	libxfs_btree_del_cursor(btr->cur, 0);
	free_slab_cursor(&btr->slab_cursor);
}

static xfs_extlen_t
estimate_allocbt_blocks(
	struct xfs_perag	*pag,
	unsigned int		nr_extents)
{
	/* Account for space consumed by both free space btrees */
	return libxfs_allocbt_calc_size(pag->pag_mount, nr_extents) * 2;
}

static xfs_extlen_t
estimate_inobt_blocks(
	struct xfs_perag	*pag)
{
	struct ino_tree_node	*ino_rec;
	xfs_agnumber_t		agno = pag->pag_agno;
	unsigned int		ino_recs = 0;
	unsigned int		fino_recs = 0;
	xfs_extlen_t		ret;

	for (ino_rec = findfirst_inode_rec(agno);
	     ino_rec != NULL;
	     ino_rec = next_ino_rec(ino_rec))  {
		unsigned int	rec_nfinos = 0;
		int		i;

		for (i = 0; i < XFS_INODES_PER_CHUNK; i++)  {
			ASSERT(is_inode_confirmed(ino_rec, i));
			/*
			 * sparse inodes are not factored into superblock (free)
			 * inode counts
			 */
			if (is_inode_sparse(ino_rec, i))
				continue;
			if (is_inode_free(ino_rec, i))
				rec_nfinos++;
		}

		ino_recs++;

		/* finobt only considers records with free inodes */
		if (rec_nfinos)
			fino_recs++;
	}

	ret = libxfs_iallocbt_calc_size(pag->pag_mount, ino_recs);
	if (xfs_has_finobt(pag->pag_mount))
		ret += libxfs_iallocbt_calc_size(pag->pag_mount, fino_recs);
	return ret;

}

/* Estimate the size of the per-AG btrees. */
xfs_extlen_t
estimate_agbtree_blocks(
	struct xfs_perag	*pag,
	unsigned int		free_extents)
{
	unsigned int		ret = 0;

	ret += estimate_allocbt_blocks(pag, free_extents);
	ret += estimate_inobt_blocks(pag);
	ret += estimate_rmapbt_blocks(pag);
	ret += estimate_refcountbt_blocks(pag);

	return ret;
}
