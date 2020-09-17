// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "list.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "scrub.h"
#include "repair.h"

/* Phase 2: Check internal metadata. */

/* Scrub each AG's metadata btrees. */
static void
scan_ag_metadata(
	struct workqueue		*wq,
	xfs_agnumber_t			agno,
	void				*arg)
{
	struct scrub_ctx		*ctx = (struct scrub_ctx *)wq->wq_ctx;
	bool				*aborted = arg;
	struct action_list		alist;
	struct action_list		immediate_alist;
	unsigned long long		broken_primaries;
	unsigned long long		broken_secondaries;
	char				descr[DESCR_BUFSZ];
	int				ret;

	if (*aborted)
		return;

	action_list_init(&alist);
	action_list_init(&immediate_alist);
	snprintf(descr, DESCR_BUFSZ, _("AG %u"), agno);

	/*
	 * First we scrub and fix the AG headers, because we need
	 * them to work well enough to check the AG btrees.
	 */
	ret = scrub_ag_headers(ctx, agno, &alist);
	if (ret)
		goto err;

	/* Repair header damage. */
	ret = action_list_process_or_defer(ctx, agno, &alist);
	if (ret)
		goto err;

	/* Now scrub the AG btrees. */
	ret = scrub_ag_metadata(ctx, agno, &alist);
	if (ret)
		goto err;

	/*
	 * Figure out if we need to perform early fixing.  The only
	 * reason we need to do this is if the inobt is broken, which
	 * prevents phase 3 (inode scan) from running.  We can rebuild
	 * the inobt from rmapbt data, but if the rmapbt is broken even
	 * at this early phase then we are sunk.
	 */
	broken_secondaries = 0;
	broken_primaries = 0;
	action_list_find_mustfix(&alist, &immediate_alist,
			&broken_primaries, &broken_secondaries);
	if (broken_secondaries && !debug_tweak_on("XFS_SCRUB_FORCE_REPAIR")) {
		if (broken_primaries)
			str_info(ctx, descr,
_("Corrupt primary and secondary block mapping metadata."));
		else
			str_info(ctx, descr,
_("Corrupt secondary block mapping metadata."));
		str_info(ctx, descr,
_("Filesystem might not be repairable."));
	}

	/* Repair (inode) btree damage. */
	ret = action_list_process_or_defer(ctx, agno, &immediate_alist);
	if (ret)
		goto err;

	/* Everything else gets fixed during phase 4. */
	action_list_defer(ctx, agno, &alist);
	return;
err:
	*aborted = true;
}

/* Scrub whole-FS metadata btrees. */
static void
scan_fs_metadata(
	struct workqueue		*wq,
	xfs_agnumber_t			agno,
	void				*arg)
{
	struct scrub_ctx		*ctx = (struct scrub_ctx *)wq->wq_ctx;
	bool				*aborted = arg;
	struct action_list		alist;
	int				ret;

	if (*aborted)
		return;

	action_list_init(&alist);
	ret = scrub_fs_metadata(ctx, &alist);
	if (ret) {
		*aborted = true;
		return;
	}

	action_list_defer(ctx, agno, &alist);
}

/* Scan all filesystem metadata. */
int
phase2_func(
	struct scrub_ctx	*ctx)
{
	struct action_list	alist;
	struct workqueue	wq;
	xfs_agnumber_t		agno;
	bool			aborted = false;
	int			ret, ret2;

	ret = -workqueue_create(&wq, (struct xfs_mount *)ctx,
			scrub_nproc_workqueue(ctx));
	if (ret) {
		str_liberror(ctx, ret, _("creating scrub workqueue"));
		return ret;
	}

	/*
	 * In case we ever use the primary super scrubber to perform fs
	 * upgrades (followed by a full scrub), do that before we launch
	 * anything else.
	 */
	action_list_init(&alist);
	ret = scrub_primary_super(ctx, &alist);
	if (ret)
		goto out;
	ret = action_list_process_or_defer(ctx, 0, &alist);
	if (ret)
		goto out;

	for (agno = 0; !aborted && agno < ctx->mnt.fsgeom.agcount; agno++) {
		ret = -workqueue_add(&wq, scan_ag_metadata, agno, &aborted);
		if (ret) {
			str_liberror(ctx, ret, _("queueing per-AG scrub work"));
			goto out;
		}
	}

	if (aborted)
		goto out;

	ret = -workqueue_add(&wq, scan_fs_metadata, 0, &aborted);
	if (ret) {
		str_liberror(ctx, ret, _("queueing per-FS scrub work"));
		goto out;
	}

out:
	ret2 = -workqueue_terminate(&wq);
	if (ret2) {
		str_liberror(ctx, ret2, _("finishing scrub work"));
		if (!ret && ret2)
			ret = ret2;
	}
	workqueue_destroy(&wq);

	if (!ret && aborted)
		ret = ECANCELED;
	return ret;
}

/* Estimate how much work we're going to do. */
int
phase2_estimate(
	struct scrub_ctx	*ctx,
	uint64_t		*items,
	unsigned int		*nr_threads,
	int			*rshift)
{
	*items = scrub_estimate_ag_work(ctx);
	*nr_threads = scrub_nproc(ctx);
	*rshift = 0;
	return 0;
}
