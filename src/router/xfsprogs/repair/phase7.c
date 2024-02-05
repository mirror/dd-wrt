// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "avl.h"
#include "globals.h"
#include "agheader.h"
#include "incore.h"
#include "protos.h"
#include "err_protos.h"
#include "dinode.h"
#include "versions.h"
#include "progress.h"
#include "threads.h"
#include "quotacheck.h"

static void
update_inode_nlinks(
	xfs_mount_t 		*mp,
	xfs_ino_t		ino,
	uint32_t		nlinks)
{
	xfs_trans_t		*tp;
	xfs_inode_t		*ip;
	int			error;
	int			dirty;
	int			nres;

	nres = no_modify ? 0 : 10;
	error = -libxfs_trans_alloc(mp, &M_RES(mp)->tr_remove, nres, 0, 0, &tp);
	ASSERT(error == 0);

	error = -libxfs_iget(mp, tp, ino, 0, &ip);
	if (error)  {
		if (!no_modify)
			do_error(
	_("couldn't map inode %" PRIu64 ", err = %d\n"),
				ino, error);
		else  {
			do_warn(
	_("couldn't map inode %" PRIu64 ", err = %d, can't compare link counts\n"),
				ino, error);
			return;
		}
	}

	dirty = 0;

	/* compare and set links if they differ.  */
	if (VFS_I(ip)->i_nlink != nlinks) {
		if (!no_modify) {
			do_warn(
	_("resetting inode %" PRIu64 " nlinks from %u to %u\n"),
				ino, VFS_I(ip)->i_nlink, nlinks);
			set_nlink(VFS_I(ip), nlinks);
			dirty = 1;
		} else {
			do_warn(
	_("would have reset inode %" PRIu64 " nlinks from %u to %u\n"),
				ino, VFS_I(ip)->i_nlink, nlinks);
		}
	}

	if (!dirty)  {
		libxfs_trans_cancel(tp);
	} else  {
		libxfs_trans_ijoin(tp, ip, 0);
		libxfs_trans_log_inode(tp, ip, XFS_ILOG_CORE);
		/*
		 * no need to do a bmap finish since
		 * we're not allocating anything
		 */
		ASSERT(error == 0);
		error = -libxfs_trans_commit(tp);

		ASSERT(error == 0);
	}
	libxfs_irele(ip);
}

/*
 * for each ag, look at each inode 1 at a time. If the number of
 * links is bad, reset it, log the inode core, commit the transaction
 */
static void
do_link_updates(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct xfs_mount	*mp = wq->wq_ctx;
	ino_tree_node_t		*irec;
	int			j;
	uint32_t		nrefs;

	for (irec = findfirst_inode_rec(agno); irec;
	     irec = next_ino_rec(irec)) {
		xfs_ino_t	ino;

		ino = XFS_AGINO_TO_INO(mp, agno, irec->ino_startnum);

		for (j = 0; j < XFS_INODES_PER_CHUNK; j++)  {
			ASSERT(is_inode_confirmed(irec, j));

			if (is_inode_free(irec, j))
				continue;

			ASSERT(no_modify || is_inode_reached(irec, j));

			nrefs = num_inode_references(irec, j);
			ASSERT(no_modify || nrefs > 0);

			if (get_inode_disk_nlinks(irec, j) != nrefs)
				update_inode_nlinks(wq->wq_ctx, ino + j, nrefs);
			quotacheck_adjust(mp, ino + j);
		}
	}

	PROG_RPT_INC(prog_rpt_done[agno], 1);
}

void
phase7(
	struct xfs_mount	*mp,
	int			scan_threads)
{
	struct workqueue	wq;
	int			agno;
	int			ret;

	if (!no_modify)
		do_log(_("Phase 7 - verify and correct link counts...\n"));
	else
		do_log(_("Phase 7 - verify link counts...\n"));

	set_progress_msg(PROGRESS_FMT_CORR_LINK, (uint64_t) glob_agcount);

	ret = quotacheck_setup(mp);
	if (ret)
		do_error(_("unable to set up quotacheck, err=%d\n"), ret);
	create_work_queue(&wq, mp, scan_threads);

	for (agno = 0; agno < mp->m_sb.sb_agcount; agno++)
		queue_work(&wq, do_link_updates, agno, NULL);

	destroy_work_queue(&wq);

	quotacheck_verify(mp, XFS_DQTYPE_USER);
	quotacheck_verify(mp, XFS_DQTYPE_GROUP);
	quotacheck_verify(mp, XFS_DQTYPE_PROJ);

	quotacheck_teardown();

	print_final_rpt();
}
