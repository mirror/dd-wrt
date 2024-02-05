// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "libfrog/bitmap.h"
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "rt.h"
#include "versions.h"
#include "threads.h"
#include "progress.h"
#include "slab.h"
#include "rmap.h"
#include "bulkload.h"
#include "agbtree.h"

static uint64_t	*sb_icount_ag;		/* allocated inodes per ag */
static uint64_t	*sb_ifree_ag;		/* free inodes per ag */
static uint64_t	*sb_fdblocks_ag;	/* free data blocks per ag */

static int
mk_incore_fstree(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	unsigned int		*num_freeblocks)
{
	int			in_extent;
	int			num_extents;
	xfs_agblock_t		extent_start;
	xfs_extlen_t		extent_len;
	xfs_agblock_t		agbno;
	xfs_agblock_t		ag_end;
	uint			free_blocks;
	xfs_extlen_t		blen;
	int			bstate;

	*num_freeblocks = 0;

	/*
	 * scan the bitmap for the ag looking for continuous
	 * extents of free blocks.  At this point, we know
	 * that blocks in the bitmap are either set to an
	 * "in use" state or set to unknown (0) since the
	 * bmaps were zero'ed in phase 4 and only blocks
	 * being used by inodes, inode bmaps, ag headers,
	 * and the files themselves were put into the bitmap.
	 *
	 */
	ASSERT(agno < mp->m_sb.sb_agcount);

	extent_start = extent_len = 0;
	in_extent = 0;
	num_extents = free_blocks = 0;

	if (agno < mp->m_sb.sb_agcount - 1)
		ag_end = mp->m_sb.sb_agblocks;
	else
		ag_end = mp->m_sb.sb_dblocks -
			(xfs_rfsblock_t)mp->m_sb.sb_agblocks *
                       (mp->m_sb.sb_agcount - 1);

	/*
	 * ok, now find the number of extents, keep track of the
	 * largest extent.
	 */
	for (agbno = 0; agbno < ag_end; agbno += blen) {
		bstate = get_bmap_ext(agno, agbno, ag_end, &blen);
		if (bstate < XR_E_INUSE)  {
			free_blocks += blen;
			if (in_extent == 0)  {
				/*
				 * found the start of a free extent
				 */
				in_extent = 1;
				num_extents++;
				extent_start = agbno;
				extent_len = blen;
			} else  {
				extent_len += blen;
			}
		} else   {
			if (in_extent)  {
				/*
				 * free extent ends here, add extent to the
				 * 2 incore extent (avl-to-be-B+) trees
				 */
				in_extent = 0;
#if defined(XR_BLD_FREE_TRACE) && defined(XR_BLD_ADD_EXTENT)
				fprintf(stderr, "adding extent %u [%u %u]\n",
					agno, extent_start, extent_len);
#endif
				add_bno_extent(agno, extent_start, extent_len);
				add_bcnt_extent(agno, extent_start, extent_len);
				*num_freeblocks += extent_len;
			}
		}
	}
	if (in_extent)  {
		/*
		 * free extent ends here
		 */
#if defined(XR_BLD_FREE_TRACE) && defined(XR_BLD_ADD_EXTENT)
		fprintf(stderr, "adding extent %u [%u %u]\n",
			agno, extent_start, extent_len);
#endif
		add_bno_extent(agno, extent_start, extent_len);
		add_bcnt_extent(agno, extent_start, extent_len);
		*num_freeblocks += extent_len;
	}

	return(num_extents);
}

/*
 * XXX: yet more code that can be shared with mkfs, growfs.
 */
static void
build_agi(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr_ino,
	struct bt_rebuild	*btr_fino)
{
	struct xfs_buf		*agi_buf;
	struct xfs_agi		*agi;
	int			i;
	int			error;

	error = -libxfs_buf_get(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGI_DADDR(mp)),
			mp->m_sb.sb_sectsize / BBSIZE, &agi_buf);
	if (error)
		do_error(_("Cannot grab AG %u AGI buffer, err=%d"),
				agno, error);
	agi_buf->b_ops = &xfs_agi_buf_ops;
	agi = agi_buf->b_addr;
	memset(agi, 0, mp->m_sb.sb_sectsize);

	agi->agi_magicnum = cpu_to_be32(XFS_AGI_MAGIC);
	agi->agi_versionnum = cpu_to_be32(XFS_AGI_VERSION);
	agi->agi_seqno = cpu_to_be32(agno);
	if (agno < mp->m_sb.sb_agcount - 1)
		agi->agi_length = cpu_to_be32(mp->m_sb.sb_agblocks);
	else
		agi->agi_length = cpu_to_be32(mp->m_sb.sb_dblocks -
			(xfs_rfsblock_t) mp->m_sb.sb_agblocks * agno);
	agi->agi_count = cpu_to_be32(btr_ino->count);
	agi->agi_root = cpu_to_be32(btr_ino->newbt.afake.af_root);
	agi->agi_level = cpu_to_be32(btr_ino->newbt.afake.af_levels);
	agi->agi_freecount = cpu_to_be32(btr_ino->freecount);
	agi->agi_newino = cpu_to_be32(btr_ino->first_agino);
	agi->agi_dirino = cpu_to_be32(NULLAGINO);

	for (i = 0; i < XFS_AGI_UNLINKED_BUCKETS; i++)
		agi->agi_unlinked[i] = cpu_to_be32(NULLAGINO);

	if (xfs_has_crc(mp))
		platform_uuid_copy(&agi->agi_uuid, &mp->m_sb.sb_meta_uuid);

	if (xfs_has_finobt(mp)) {
		agi->agi_free_root =
				cpu_to_be32(btr_fino->newbt.afake.af_root);
		agi->agi_free_level =
				cpu_to_be32(btr_fino->newbt.afake.af_levels);
	}

	if (xfs_has_inobtcounts(mp)) {
		agi->agi_iblocks = cpu_to_be32(btr_ino->newbt.afake.af_blocks);
		agi->agi_fblocks = cpu_to_be32(btr_fino->newbt.afake.af_blocks);
	}

	libxfs_buf_mark_dirty(agi_buf);
	libxfs_buf_relse(agi_buf);
}

/* Fill the AGFL with any leftover bnobt rebuilder blocks. */
static void
fill_agfl(
	struct bt_rebuild	*btr,
	__be32			*agfl_bnos,
	unsigned int		*agfl_idx)
{
	struct bulkload_resv	*resv, *n;
	struct xfs_mount	*mp = btr->newbt.sc->mp;

	for_each_bulkload_reservation(&btr->newbt, resv, n) {
		xfs_agblock_t	bno;

		bno = XFS_FSB_TO_AGBNO(mp, resv->fsbno + resv->used);
		while (resv->used < resv->len &&
		       *agfl_idx < libxfs_agfl_size(mp)) {
			agfl_bnos[(*agfl_idx)++] = cpu_to_be32(bno++);
			resv->used++;
		}
	}
}

/*
 * build both the agf and the agfl for an agno given both
 * btree cursors.
 *
 * XXX: yet more common code that can be shared with mkfs/growfs.
 */
static void
build_agf_agfl(
	struct xfs_mount	*mp,
	xfs_agnumber_t		agno,
	struct bt_rebuild	*btr_bno,
	struct bt_rebuild	*btr_cnt,
	struct bt_rebuild	*btr_rmap,
	struct bt_rebuild	*btr_refc,
	struct bitmap		*lost_blocks)
{
	struct extent_tree_node	*ext_ptr;
	struct xfs_buf		*agf_buf, *agfl_buf;
	unsigned int		agfl_idx;
	struct xfs_agfl		*agfl;
	struct xfs_agf		*agf;
	__be32			*freelist;
	int			error;

	error = -libxfs_buf_get(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGF_DADDR(mp)),
			mp->m_sb.sb_sectsize / BBSIZE, &agf_buf);
	if (error)
		do_error(_("Cannot grab AG %u AGF buffer, err=%d"),
				agno, error);
	agf_buf->b_ops = &xfs_agf_buf_ops;
	agf = agf_buf->b_addr;
	memset(agf, 0, mp->m_sb.sb_sectsize);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "agf = %p, agf_buf->b_addr = %p\n",
		agf, agf_buf->b_addr);
#endif

	/*
	 * set up fixed part of agf
	 */
	agf->agf_magicnum = cpu_to_be32(XFS_AGF_MAGIC);
	agf->agf_versionnum = cpu_to_be32(XFS_AGF_VERSION);
	agf->agf_seqno = cpu_to_be32(agno);

	if (agno < mp->m_sb.sb_agcount - 1)
		agf->agf_length = cpu_to_be32(mp->m_sb.sb_agblocks);
	else
		agf->agf_length = cpu_to_be32(mp->m_sb.sb_dblocks -
			(xfs_rfsblock_t) mp->m_sb.sb_agblocks * agno);

	agf->agf_roots[XFS_BTNUM_BNO] =
			cpu_to_be32(btr_bno->newbt.afake.af_root);
	agf->agf_levels[XFS_BTNUM_BNO] =
			cpu_to_be32(btr_bno->newbt.afake.af_levels);
	agf->agf_roots[XFS_BTNUM_CNT] =
			cpu_to_be32(btr_cnt->newbt.afake.af_root);
	agf->agf_levels[XFS_BTNUM_CNT] =
			cpu_to_be32(btr_cnt->newbt.afake.af_levels);
	agf->agf_freeblks = cpu_to_be32(btr_bno->freeblks);

	if (xfs_has_rmapbt(mp)) {
		agf->agf_roots[XFS_BTNUM_RMAP] =
				cpu_to_be32(btr_rmap->newbt.afake.af_root);
		agf->agf_levels[XFS_BTNUM_RMAP] =
				cpu_to_be32(btr_rmap->newbt.afake.af_levels);
		agf->agf_rmap_blocks =
				cpu_to_be32(btr_rmap->newbt.afake.af_blocks);
	}

	if (xfs_has_reflink(mp)) {
		agf->agf_refcount_root =
				cpu_to_be32(btr_refc->newbt.afake.af_root);
		agf->agf_refcount_level =
				cpu_to_be32(btr_refc->newbt.afake.af_levels);
		agf->agf_refcount_blocks =
				cpu_to_be32(btr_refc->newbt.afake.af_blocks);
	}

	/*
	 * Count and record the number of btree blocks consumed if required.
	 */
	if (xfs_has_lazysbcount(mp)) {
		unsigned int blks;
		/*
		 * Don't count the root blocks as they are already
		 * accounted for.
		 */
		blks = btr_bno->newbt.afake.af_blocks +
			btr_cnt->newbt.afake.af_blocks - 2;
		if (xfs_has_rmapbt(mp))
			blks += btr_rmap->newbt.afake.af_blocks - 1;
		agf->agf_btreeblks = cpu_to_be32(blks);
#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "agf->agf_btreeblks = %u\n",
				be32_to_cpu(agf->agf_btreeblks));
#endif
	}

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "bno root = %u, bcnt root = %u, indices = %u %u\n",
			be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNO]),
			be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNT]),
			XFS_BTNUM_BNO,
			XFS_BTNUM_CNT);
#endif

	if (xfs_has_crc(mp))
		platform_uuid_copy(&agf->agf_uuid, &mp->m_sb.sb_meta_uuid);

	/* initialise the AGFL, then fill it if there are blocks left over. */
	error = -libxfs_buf_get(mp->m_dev,
			XFS_AG_DADDR(mp, agno, XFS_AGFL_DADDR(mp)),
			mp->m_sb.sb_sectsize / BBSIZE, &agfl_buf);
	if (error)
		do_error(_("Cannot grab AG %u AGFL buffer, err=%d"),
				agno, error);
	agfl_buf->b_ops = &xfs_agfl_buf_ops;
	agfl = XFS_BUF_TO_AGFL(agfl_buf);

	/* setting to 0xff results in initialisation to NULLAGBLOCK */
	memset(agfl, 0xff, mp->m_sb.sb_sectsize);
	freelist = xfs_buf_to_agfl_bno(agfl_buf);
	if (xfs_has_crc(mp)) {
		agfl->agfl_magicnum = cpu_to_be32(XFS_AGFL_MAGIC);
		agfl->agfl_seqno = cpu_to_be32(agno);
		platform_uuid_copy(&agfl->agfl_uuid, &mp->m_sb.sb_meta_uuid);
		for (agfl_idx = 0; agfl_idx < libxfs_agfl_size(mp); agfl_idx++)
			freelist[agfl_idx] = cpu_to_be32(NULLAGBLOCK);
	}

	/* Fill the AGFL with leftover blocks or save them for later. */
	agfl_idx = 0;
	freelist = xfs_buf_to_agfl_bno(agfl_buf);
	fill_agfl(btr_bno, freelist, &agfl_idx);
	fill_agfl(btr_cnt, freelist, &agfl_idx);
	if (xfs_has_rmapbt(mp))
		fill_agfl(btr_rmap, freelist, &agfl_idx);

	/* Set the AGF counters for the AGFL. */
	if (agfl_idx > 0) {
		agf->agf_flfirst = 0;
		agf->agf_fllast = cpu_to_be32(agfl_idx - 1);
		agf->agf_flcount = cpu_to_be32(agfl_idx);
		rmap_store_agflcount(mp, agno, agfl_idx);

#ifdef XR_BLD_FREE_TRACE
		fprintf(stderr, "writing agfl for ag %u\n", agno);
#endif

	} else  {
		agf->agf_flfirst = 0;
		agf->agf_fllast = cpu_to_be32(libxfs_agfl_size(mp) - 1);
		agf->agf_flcount = 0;
	}

	libxfs_buf_mark_dirty(agfl_buf);
	libxfs_buf_relse(agfl_buf);

	ext_ptr = findbiggest_bcnt_extent(agno);
	agf->agf_longest = cpu_to_be32((ext_ptr != NULL) ?
						ext_ptr->ex_blockcount : 0);

	ASSERT(be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNOi]) !=
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNTi]));
	ASSERT(be32_to_cpu(agf->agf_refcount_root) !=
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_BNOi]));
	ASSERT(be32_to_cpu(agf->agf_refcount_root) !=
		be32_to_cpu(agf->agf_roots[XFS_BTNUM_CNTi]));

	libxfs_buf_mark_dirty(agf_buf);
	libxfs_buf_relse(agf_buf);

	/*
	 * now fix up the free list appropriately
	 */
	fix_freelist(mp, agno, true);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "wrote agf for ag %u\n", agno);
#endif
}

/*
 * update the superblock counters, sync the sb version numbers and
 * feature bits to the filesystem, and sync up the on-disk superblock
 * to match the incore superblock.
 */
static void
sync_sb(xfs_mount_t *mp)
{
	struct xfs_buf	*bp;

	bp = libxfs_getsb(mp);
	if (!bp)
		do_error(_("couldn't get superblock\n"));

	mp->m_sb.sb_icount = sb_icount;
	mp->m_sb.sb_ifree = sb_ifree;
	mp->m_sb.sb_fdblocks = sb_fdblocks;
	mp->m_sb.sb_frextents = sb_frextents;

	update_sb_version(mp);

	libxfs_sb_to_disk(bp->b_addr, &mp->m_sb);
	libxfs_buf_mark_dirty(bp);
	libxfs_buf_relse(bp);
}

/*
 * make sure the root and realtime inodes show up allocated
 * even if they've been freed.  they get reinitialized in phase6.
 */
static void
keep_fsinos(xfs_mount_t *mp)
{
	ino_tree_node_t		*irec;
	int			i;

	irec = find_inode_rec(mp, XFS_INO_TO_AGNO(mp, mp->m_sb.sb_rootino),
			XFS_INO_TO_AGINO(mp, mp->m_sb.sb_rootino));

	for (i = 0; i < 3; i++)
		set_inode_used(irec, i);
}

static void
phase5_func(
	struct xfs_mount	*mp,
	struct xfs_perag	*pag,
	struct bitmap		*lost_blocks)
{
	struct repair_ctx	sc = { .mp = mp, };
	struct bt_rebuild	btr_bno;
	struct bt_rebuild	btr_cnt;
	struct bt_rebuild	btr_ino;
	struct bt_rebuild	btr_fino;
	struct bt_rebuild	btr_rmap;
	struct bt_rebuild	btr_refc;
	xfs_agnumber_t		agno = pag->pag_agno;
	int			extra_blocks = 0;
	uint			num_freeblocks;
	xfs_agblock_t		num_extents;
	unsigned int		est_agfreeblocks = 0;
	unsigned int		total_btblocks;

	if (verbose)
		do_log(_("        - agno = %d\n"), agno);

	/*
	 * build up incore bno and bcnt extent btrees
	 */
	num_extents = mk_incore_fstree(mp, agno, &num_freeblocks);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "# of bno extents is %d\n", count_bno_extents(agno));
#endif

	if (num_extents == 0)  {
		/*
		 * XXX - what we probably should do here is pick an inode for
		 * a regular file in the allocation group that has space
		 * allocated and shoot it by traversing the bmap list and
		 * putting all its extents on the incore freespace trees,
		 * clearing the inode, and clearing the in-use bit in the
		 * incore inode tree.  Then try mk_incore_fstree() again.
		 */
		do_error(
_("unable to rebuild AG %u.  Not enough free space in on-disk AG.\n"),
			agno);
	}

	/*
	 * Estimate the number of free blocks in this AG after rebuilding
	 * all btrees.
	 */
	total_btblocks = estimate_agbtree_blocks(pag, num_extents);
	if (num_freeblocks > total_btblocks)
		est_agfreeblocks = num_freeblocks - total_btblocks;

	init_ino_cursors(&sc, pag, est_agfreeblocks, &sb_icount_ag[agno],
			&sb_ifree_ag[agno], &btr_ino, &btr_fino);

	init_rmapbt_cursor(&sc, pag, est_agfreeblocks, &btr_rmap);

	init_refc_cursor(&sc, pag, est_agfreeblocks, &btr_refc);

	num_extents = count_bno_extents_blocks(agno, &num_freeblocks);
	/*
	 * lose two blocks per AG -- the space tree roots are counted as
	 * allocated since the space trees always have roots
	 */
	sb_fdblocks_ag[agno] += num_freeblocks - 2;

	if (num_extents == 0)  {
		/*
		 * XXX - what we probably should do here is pick an inode for
		 * a regular file in the allocation group that has space
		 * allocated and shoot it by traversing the bmap list and
		 * putting all its extents on the incore freespace trees,
		 * clearing the inode, and clearing the in-use bit in the
		 * incore inode tree.  Then try mk_incore_fstree() again.
		 */
		do_error(_("unable to rebuild AG %u.  No free space.\n"), agno);
	}

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "# of bno extents is %d\n", num_extents);
#endif

	/*
	 * track blocks that we might really lose
	 */
	init_freespace_cursors(&sc, pag, est_agfreeblocks, &num_extents,
			&extra_blocks, &btr_bno, &btr_cnt);

	/*
	 * freespace btrees live in the "free space" but the filesystem treats
	 * AGFL blocks as allocated since they aren't described by the
	 * freespace trees
	 */

	/*
	 * see if we can fit all the extra blocks into the AGFL
	 */
	extra_blocks = (extra_blocks - libxfs_agfl_size(mp) > 0) ?
			extra_blocks - libxfs_agfl_size(mp) : 0;

	if (extra_blocks > 0)
		sb_fdblocks_ag[agno] -= extra_blocks;

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "# of bno extents is %d\n", count_bno_extents(agno));
	fprintf(stderr, "# of bcnt extents is %d\n", count_bcnt_extents(agno));
#endif

	build_freespace_btrees(&sc, agno, &btr_bno, &btr_cnt);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "# of free blocks == %d/%d\n", btr_bno.freeblks,
			btr_cnt.freeblks);
#endif
	ASSERT(btr_bno.freeblks == btr_cnt.freeblks);

	if (xfs_has_rmapbt(mp)) {
		build_rmap_tree(&sc, agno, &btr_rmap);
		sb_fdblocks_ag[agno] += btr_rmap.newbt.afake.af_blocks - 1;
	}

	if (xfs_has_reflink(mp))
		build_refcount_tree(&sc, agno, &btr_refc);

	/*
	 * set up agf and agfl
	 */
	build_agf_agfl(mp, agno, &btr_bno, &btr_cnt, &btr_rmap, &btr_refc,
			lost_blocks);

	build_inode_btrees(&sc, agno, &btr_ino, &btr_fino);

	/* build the agi */
	build_agi(mp, agno, &btr_ino, &btr_fino);

	/*
	 * tear down cursors
	 */
	finish_rebuild(mp, &btr_bno, lost_blocks);
	finish_rebuild(mp, &btr_cnt, lost_blocks);
	finish_rebuild(mp, &btr_ino, lost_blocks);
	if (xfs_has_finobt(mp))
		finish_rebuild(mp, &btr_fino, lost_blocks);
	if (xfs_has_rmapbt(mp))
		finish_rebuild(mp, &btr_rmap, lost_blocks);
	if (xfs_has_reflink(mp))
		finish_rebuild(mp, &btr_refc, lost_blocks);

	/*
	 * release the incore per-AG bno/bcnt trees so the extent nodes
	 * can be recycled
	 */
	release_agbno_extent_tree(agno);
	release_agbcnt_extent_tree(agno);
	PROG_RPT_INC(prog_rpt_done[agno], 1);
}

/* Inject this unused space back into the filesystem. */
static int
inject_lost_extent(
	uint64_t		start,
	uint64_t		length,
	void			*arg)
{
	struct xfs_mount	*mp = arg;
	struct xfs_trans	*tp;
	struct xfs_perag	*pag;
	xfs_agnumber_t		agno;
	xfs_agblock_t		agbno;
	int			error;

	error = -libxfs_trans_alloc_rollable(mp, 16, &tp);
	if (error)
		return error;

	agno = XFS_FSB_TO_AGNO(mp, start);
	agbno = XFS_FSB_TO_AGBNO(mp, start);
	pag = libxfs_perag_get(mp, agno);
	error = -libxfs_free_extent(tp, pag, agbno, length,
			&XFS_RMAP_OINFO_ANY_OWNER, XFS_AG_RESV_NONE);
	libxfs_perag_put(pag);

	if (error)
		return error;

	return -libxfs_trans_commit(tp);
}

void
check_rtmetadata(
	struct xfs_mount	*mp)
{
	rtinit(mp);
	generate_rtinfo(mp, btmcompute, sumcompute);
	check_rtbitmap(mp);
	check_rtsummary(mp);
}

void
phase5(xfs_mount_t *mp)
{
	struct bitmap		*lost_blocks = NULL;
	struct xfs_perag	*pag;
	xfs_agnumber_t		agno;
	int			error;

	do_log(_("Phase 5 - rebuild AG headers and trees...\n"));
	set_progress_msg(PROG_FMT_REBUILD_AG, (uint64_t)glob_agcount);

#ifdef XR_BLD_FREE_TRACE
	fprintf(stderr, "inobt level 1, maxrec = %d, minrec = %d\n",
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 0),
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 0) / 2);
	fprintf(stderr, "inobt level 0 (leaf), maxrec = %d, minrec = %d\n",
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 1),
		libxfs_inobt_maxrecs(mp, mp->m_sb.sb_blocksize, 1) / 2);
	fprintf(stderr, "xr inobt level 0 (leaf), maxrec = %d\n",
		XR_INOBT_BLOCK_MAXRECS(mp, 0));
	fprintf(stderr, "xr inobt level 1 (int), maxrec = %d\n",
		XR_INOBT_BLOCK_MAXRECS(mp, 1));
	fprintf(stderr, "bnobt level 1, maxrec = %d, minrec = %d\n",
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 0),
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 0) / 2);
	fprintf(stderr, "bnobt level 0 (leaf), maxrec = %d, minrec = %d\n",
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 1),
		libxfs_allocbt_maxrecs(mp, mp->m_sb.sb_blocksize, 1) / 2);
#endif
	/*
	 * make sure the root and realtime inodes show up allocated
	 */
	keep_fsinos(mp);

	/* allocate per ag counters */
	sb_icount_ag = calloc(mp->m_sb.sb_agcount, sizeof(uint64_t));
	if (sb_icount_ag == NULL)
		do_error(_("cannot alloc sb_icount_ag buffers\n"));

	sb_ifree_ag = calloc(mp->m_sb.sb_agcount, sizeof(uint64_t));
	if (sb_ifree_ag == NULL)
		do_error(_("cannot alloc sb_ifree_ag buffers\n"));

	sb_fdblocks_ag = calloc(mp->m_sb.sb_agcount, sizeof(uint64_t));
	if (sb_fdblocks_ag == NULL)
		do_error(_("cannot alloc sb_fdblocks_ag buffers\n"));

	error = bitmap_alloc(&lost_blocks);
	if (error)
		do_error(_("cannot alloc lost block bitmap\n"));

	for_each_perag(mp, agno, pag)
		phase5_func(mp, pag, lost_blocks);

	print_final_rpt();

	/* aggregate per ag counters */
	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)  {
		sb_icount += sb_icount_ag[agno];
		sb_ifree += sb_ifree_ag[agno];
		sb_fdblocks += sb_fdblocks_ag[agno];
	}
	free(sb_icount_ag);
	free(sb_ifree_ag);
	free(sb_fdblocks_ag);

	if (mp->m_sb.sb_rblocks)  {
		do_log(
		_("        - generate realtime summary info and bitmap...\n"));
		check_rtmetadata(mp);
	}

	do_log(_("        - reset superblock...\n"));

	/*
	 * sync superblock counter and set version bits correctly
	 */
	sync_sb(mp);

	/*
	 * Put the per-AG btree rmap data into the rmapbt now that we've reset
	 * the superblock counters.
	 */
	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++) {
		error = rmap_store_ag_btree_rec(mp, agno);
		if (error)
			do_error(
_("unable to add AG %u reverse-mapping data to btree.\n"), agno);
	}

	/*
	 * Put blocks that were unnecessarily reserved for btree
	 * reconstruction back into the filesystem free space data.
	 */
	error = bitmap_iterate(lost_blocks, inject_lost_extent, mp);
	if (error)
		do_error(_("Unable to reinsert lost blocks into filesystem.\n"));
	bitmap_free(&lost_blocks);

	bad_ino_btree = 0;

}
