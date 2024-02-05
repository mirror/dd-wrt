// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2002,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "threads.h"
#include "prefetch.h"
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "bmap.h"
#include "versions.h"
#include "dir2.h"
#include "progress.h"
#include "slab.h"
#include "rmap.h"

bool collect_rmaps;

/*
 * null out quota inode fields in sb if they point to non-existent inodes.
 * this isn't as redundant as it looks since it's possible that the sb field
 * might be set but the imap and inode(s) agree that the inode is
 * free in which case they'd never be cleared so the fields wouldn't
 * be cleared by process_dinode().
 */
static void
quotino_check(xfs_mount_t *mp)
{
	ino_tree_node_t *irec;

	if (mp->m_sb.sb_uquotino != NULLFSINO && mp->m_sb.sb_uquotino != 0)  {
		if (!libxfs_verify_ino(mp, mp->m_sb.sb_uquotino))
			irec = NULL;
		else
			irec = find_inode_rec(mp,
				XFS_INO_TO_AGNO(mp, mp->m_sb.sb_uquotino),
				XFS_INO_TO_AGINO(mp, mp->m_sb.sb_uquotino));

		if (irec == NULL || is_inode_free(irec,
				mp->m_sb.sb_uquotino - irec->ino_startnum))  {
			mp->m_sb.sb_uquotino = NULLFSINO;
			lost_uquotino = 1;
		} else
			lost_uquotino = 0;
	}

	if (mp->m_sb.sb_gquotino != NULLFSINO && mp->m_sb.sb_gquotino != 0)  {
		if (!libxfs_verify_ino(mp, mp->m_sb.sb_gquotino))
			irec = NULL;
		else
			irec = find_inode_rec(mp,
				XFS_INO_TO_AGNO(mp, mp->m_sb.sb_gquotino),
				XFS_INO_TO_AGINO(mp, mp->m_sb.sb_gquotino));

		if (irec == NULL || is_inode_free(irec,
				mp->m_sb.sb_gquotino - irec->ino_startnum))  {
			mp->m_sb.sb_gquotino = NULLFSINO;
			lost_gquotino = 1;
		} else
			lost_gquotino = 0;
	}

	if (mp->m_sb.sb_pquotino != NULLFSINO && mp->m_sb.sb_pquotino != 0)  {
		if (!libxfs_verify_ino(mp, mp->m_sb.sb_pquotino))
			irec = NULL;
		else
			irec = find_inode_rec(mp,
				XFS_INO_TO_AGNO(mp, mp->m_sb.sb_pquotino),
				XFS_INO_TO_AGINO(mp, mp->m_sb.sb_pquotino));

		if (irec == NULL || is_inode_free(irec,
				mp->m_sb.sb_pquotino - irec->ino_startnum))  {
			mp->m_sb.sb_pquotino = NULLFSINO;
			lost_pquotino = 1;
		} else
			lost_pquotino = 0;
	}
}

static void
quota_sb_check(xfs_mount_t *mp)
{
	/*
	 * if the sb says we have quotas and we lost both,
	 * signal a superblock downgrade.  that will cause
	 * the quota flags to get zeroed.  (if we only lost
	 * one quota inode, do nothing and complain later.)
	 *
	 * if the sb says we have quotas but we didn't start out
	 * with any quota inodes, signal a superblock downgrade.
	 *
	 * The sb downgrades are so that older systems can mount
	 * the filesystem.
	 *
	 * if the sb says we don't have quotas but it looks like
	 * we do have quota inodes, then signal a superblock upgrade.
	 *
	 * if the sb says we don't have quotas and we have no
	 * quota inodes, then leave will enough alone.
	 */

	if (fs_quotas &&
	    (mp->m_sb.sb_uquotino == NULLFSINO || mp->m_sb.sb_uquotino == 0) &&
	    (mp->m_sb.sb_gquotino == NULLFSINO || mp->m_sb.sb_gquotino == 0) &&
	    (mp->m_sb.sb_pquotino == NULLFSINO || mp->m_sb.sb_pquotino == 0))  {
		lost_quotas = 1;
		fs_quotas = 0;
	} else if (libxfs_verify_ino(mp, mp->m_sb.sb_uquotino) &&
		   libxfs_verify_ino(mp, mp->m_sb.sb_gquotino) &&
		   libxfs_verify_ino(mp, mp->m_sb.sb_pquotino)) {
		fs_quotas = 1;
	}
}


static void
process_ag_func(
	struct workqueue	*wq,
	xfs_agnumber_t 		agno,
	void			*arg)
{
	wait_for_inode_prefetch(arg);
	do_log(_("        - agno = %d\n"), agno);
	process_aginodes(wq->wq_ctx, arg, agno, 0, 1, 0);
	blkmap_free_final();
	cleanup_inode_prefetch(arg);

	/*
	 * now recycle the per-AG duplicate extent records
	 */
	release_dup_extent_tree(agno);
}

static void
process_ags(
	xfs_mount_t		*mp)
{
	xfs_agnumber_t		i;
	int			error;

	do_inode_prefetch(mp, ag_stride, process_ag_func, true, false);
	for (i = 0; i < mp->m_sb.sb_agcount; i++) {
		error = rmap_finish_collecting_fork_recs(mp, i);
		if (error)
			do_error(
_("unable to finish adding attr/data fork reverse-mapping data for AG %u.\n"),
				i);
	}
}

static void
check_rmap_btrees(
	struct workqueue*wq,
	xfs_agnumber_t	agno,
	void		*arg)
{
	int		error;

	error = rmap_add_fixed_ag_rec(wq->wq_ctx, agno);
	if (error)
		do_error(
_("unable to add AG %u metadata reverse-mapping data.\n"), agno);

	error = rmap_fold_raw_recs(wq->wq_ctx, agno);
	if (error)
		do_error(
_("unable to merge AG %u metadata reverse-mapping data.\n"), agno);

	rmaps_verify_btree(wq->wq_ctx, agno);
}

static void
compute_ag_refcounts(
	struct workqueue*wq,
	xfs_agnumber_t	agno,
	void		*arg)
{
	int		error;

	error = compute_refcounts(wq->wq_ctx, agno);
	if (error)
		do_error(
_("%s while computing reference count records.\n"),
			 strerror(-error));
}

static void
process_inode_reflink_flags(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	int			error;

	error = fix_inode_reflink_flags(wq->wq_ctx, agno);
	if (error)
		do_error(
_("%s while fixing inode reflink flags.\n"),
			 strerror(-error));
}

static void
check_refcount_btrees(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	check_refcounts(wq->wq_ctx, agno);
}

static void
process_rmap_data(
	struct xfs_mount	*mp)
{
	struct workqueue	wq;
	xfs_agnumber_t		i;

	if (!rmap_needs_work(mp))
		return;

	create_work_queue(&wq, mp, platform_nproc());
	for (i = 0; i < mp->m_sb.sb_agcount; i++)
		queue_work(&wq, check_rmap_btrees, i, NULL);
	destroy_work_queue(&wq);

	if (!xfs_has_reflink(mp))
		return;

	create_work_queue(&wq, mp, platform_nproc());
	for (i = 0; i < mp->m_sb.sb_agcount; i++)
		queue_work(&wq, compute_ag_refcounts, i, NULL);
	destroy_work_queue(&wq);

	create_work_queue(&wq, mp, platform_nproc());
	for (i = 0; i < mp->m_sb.sb_agcount; i++) {
		queue_work(&wq, process_inode_reflink_flags, i, NULL);
		queue_work(&wq, check_refcount_btrees, i, NULL);
	}
	destroy_work_queue(&wq);
}

void
phase4(xfs_mount_t *mp)
{
	ino_tree_node_t		*irec;
	xfs_rtblock_t		bno;
	xfs_rtblock_t		rt_start;
	xfs_extlen_t		rt_len;
	xfs_agnumber_t		i;
	xfs_agblock_t		j;
	xfs_agblock_t		ag_end;
	xfs_extlen_t		blen;
	int			ag_hdr_len = 4 * mp->m_sb.sb_sectsize;
	int			ag_hdr_block;
	int			bstate;

	if (rmap_needs_work(mp))
		collect_rmaps = true;
	ag_hdr_block = howmany(ag_hdr_len, mp->m_sb.sb_blocksize);

	do_log(_("Phase 4 - check for duplicate blocks...\n"));
	do_log(_("        - setting up duplicate extent list...\n"));

	set_progress_msg(PROG_FMT_DUP_EXTENT, (uint64_t) glob_agcount);

	irec = find_inode_rec(mp, XFS_INO_TO_AGNO(mp, mp->m_sb.sb_rootino),
				XFS_INO_TO_AGINO(mp, mp->m_sb.sb_rootino));

	/*
	 * we always have a root inode, even if it's free...
	 * if the root is free, forget it, lost+found is already gone
	 */
	if (is_inode_free(irec, 0) || !inode_isadir(irec, 0))  {
		need_root_inode = 1;
		if (no_modify)
			do_warn(_("root inode would be lost\n"));
		else
			do_warn(_("root inode lost\n"));
	}

	for (i = 0; i < mp->m_sb.sb_agcount; i++)  {
		ag_end = (i < mp->m_sb.sb_agcount - 1) ? mp->m_sb.sb_agblocks :
			mp->m_sb.sb_dblocks -
				(xfs_rfsblock_t) mp->m_sb.sb_agblocks * i;

		/*
		 * set up duplicate extent list for this ag
		 */
		for (j = ag_hdr_block; j < ag_end; j += blen)  {
			bstate = get_bmap_ext(i, j, ag_end, &blen);
			switch (bstate) {
			case XR_E_FREE1:
				if (no_modify)
					do_warn(
	_("free space (%u,%u-%u) only seen by one free space btree\n"),
						i, j, j + blen - 1);
				break;
			case XR_E_BAD_STATE:
			default:
				do_warn(
				_("unknown block state, ag %d, blocks %u-%u\n"),
					i, j, j + blen - 1);
				fallthrough;
			case XR_E_UNKNOWN:
			case XR_E_FREE:
			case XR_E_INUSE:
			case XR_E_INUSE_FS:
			case XR_E_INO:
			case XR_E_FS_MAP:
				break;
			case XR_E_MULT:
				add_dup_extent(i, j, blen);
				break;
			}
		}

		PROG_RPT_INC(prog_rpt_done[i], 1);
	}
	print_final_rpt();

	/*
	 * initialize realtime bitmap
	 */
	rt_start = 0;
	rt_len = 0;

	for (bno = 0; bno < mp->m_sb.sb_rextents; bno++)  {
		bstate = get_rtbmap(bno);
		switch (bstate)  {
		case XR_E_BAD_STATE:
		default:
			do_warn(
	_("unknown rt extent state, extent %" PRIu64 "\n"),
				bno);
			fallthrough;
		case XR_E_UNKNOWN:
		case XR_E_FREE1:
		case XR_E_FREE:
		case XR_E_INUSE:
		case XR_E_INUSE_FS:
		case XR_E_INO:
		case XR_E_FS_MAP:
			if (rt_start == 0)
				continue;
			else  {
				/*
				 * add extent and reset extent state
				 */
				add_rt_dup_extent(rt_start, rt_len);
				rt_start = 0;
				rt_len = 0;
			}
			break;
		case XR_E_MULT:
			if (rt_start == 0)  {
				rt_start = bno;
				rt_len = 1;
			} else if (rt_len == XFS_MAX_BMBT_EXTLEN)  {
				/*
				 * large extent case
				 */
				add_rt_dup_extent(rt_start, rt_len);
				rt_start = bno;
				rt_len = 1;
			} else
				rt_len++;
			break;
		}
	}

	/*
	 * catch tail-case, extent hitting the end of the ag
	 */
	if (rt_start != 0)
		add_rt_dup_extent(rt_start, rt_len);

	/*
	 * initialize bitmaps for all AGs
	 */
	reset_bmaps(mp);

	do_log(_("        - check for inodes claiming duplicate blocks...\n"));
	set_progress_msg(PROG_FMT_DUP_BLOCKS, (uint64_t) mp->m_sb.sb_icount);

	/*
	 * ok, now process the inodes -- signal 2-pass check per inode.
	 * first pass checks if the inode conflicts with a known
	 * duplicate extent.  if so, the inode is cleared and second
	 * pass is skipped.  second pass sets the block bitmap
	 * for all blocks claimed by the inode.  directory
	 * and attribute processing is turned OFF since we did that
	 * already in phase 3.
	 */
	process_ags(mp);

	/*
	 * Process all the reverse-mapping data that we collected.  This
	 * involves checking the rmap data against the btree, computing
	 * reference counts based on the rmap data, and checking the counts
	 * against the refcount btree.
	 */
	process_rmap_data(mp);

	print_final_rpt();

	/*
	 * free up memory used to track trealtime duplicate extents
	 */
	if (rt_start != 0)
		free_rt_dup_extent_tree(mp);

	/*
	 * ensure consistency of quota inode pointers in superblock,
	 * make sure they point to real inodes
	 */
	quotino_check(mp);
	quota_sb_check(mp);
}
