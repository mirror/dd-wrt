// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include "list.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "progress.h"
#include "scrub.h"
#include "repair.h"
#include "vfs.h"

/* Phase 4: Repair filesystem. */

/* Fix all the problems in our per-AG list. */
static void
repair_ag(
	struct workqueue		*wq,
	xfs_agnumber_t			agno,
	void				*priv)
{
	struct scrub_ctx		*ctx = (struct scrub_ctx *)wq->wq_ctx;
	bool				*aborted = priv;
	struct action_list		*alist;
	unsigned long long		unfixed;
	unsigned long long		new_unfixed;
	unsigned int			flags = 0;
	int				ret;

	alist = &ctx->action_lists[agno];
	unfixed = action_list_length(alist);

	/* Repair anything broken until we fail to make progress. */
	do {
		ret = action_list_process(ctx, -1, alist, flags);
		if (ret) {
			*aborted = true;
			return;
		}
		new_unfixed = action_list_length(alist);
		if (new_unfixed == unfixed)
			break;
		unfixed = new_unfixed;
		if (*aborted)
			return;
	} while (unfixed > 0);

	/* Try once more, but this time complain if we can't fix things. */
	flags |= ALP_COMPLAIN_IF_UNFIXED;
	ret = action_list_process(ctx, -1, alist, flags);
	if (ret)
		*aborted = true;
}

/* Process all the action items. */
static int
repair_everything(
	struct scrub_ctx		*ctx)
{
	struct workqueue		wq;
	xfs_agnumber_t			agno;
	bool				aborted = false;
	int				ret;

	ret = -workqueue_create(&wq, (struct xfs_mount *)ctx,
			scrub_nproc_workqueue(ctx));
	if (ret) {
		str_liberror(ctx, ret, _("creating repair workqueue"));
		return ret;
	}
	for (agno = 0; !aborted && agno < ctx->mnt.fsgeom.agcount; agno++) {
		if (action_list_length(&ctx->action_lists[agno]) == 0)
			continue;

		ret = -workqueue_add(&wq, repair_ag, agno, &aborted);
		if (ret) {
			str_liberror(ctx, ret, _("queueing repair work"));
			break;
		}
	}

	ret = -workqueue_terminate(&wq);
	if (ret)
		str_liberror(ctx, ret, _("finishing repair work"));
	workqueue_destroy(&wq);

	if (aborted)
		return ECANCELED;

	return 0;
}

/* Decide if we have any repair work to do. */
static inline bool
have_action_items(
	struct scrub_ctx	*ctx)
{
	xfs_agnumber_t		agno;

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++) {
		if (action_list_length(&ctx->action_lists[agno]) > 0)
			return true;
	}

	return false;
}

/* Trim the unused areas of the filesystem if the caller asked us to. */
static void
trim_filesystem(
	struct scrub_ctx	*ctx)
{
	if (want_fstrim)
		fstrim(ctx);
	progress_add(1);
}

/* Fix everything that needs fixing. */
int
phase4_func(
	struct scrub_ctx	*ctx)
{
	int			ret;

	if (!have_action_items(ctx))
		goto maybe_trim;

	/*
	 * Check the summary counters early.  Normally we do this during phase
	 * seven, but some of the cross-referencing requires fairly-accurate
	 * counters, so counter repairs have to be put on the list now so that
	 * they get fixed before we stop retrying unfixed metadata repairs.
	 */
	ret = scrub_fs_summary(ctx, &ctx->action_lists[0]);
	if (ret)
		return ret;

	ret = repair_everything(ctx);
	if (ret)
		return ret;

	/*
	 * If errors remain on the filesystem, do not trim anything.  We don't
	 * have any threads running, so it's ok to skip the ctx lock here.
	 */
	if (ctx->corruptions_found || ctx->unfixable_errors != 0)
		return 0;

maybe_trim:
	trim_filesystem(ctx);
	return 0;
}

/* Estimate how much work we're going to do. */
int
phase4_estimate(
	struct scrub_ctx	*ctx,
	uint64_t		*items,
	unsigned int		*nr_threads,
	int			*rshift)
{
	xfs_agnumber_t		agno;
	unsigned long long	need_fixing = 0;

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++)
		need_fixing += action_list_length(&ctx->action_lists[agno]);
	need_fixing++;
	*items = need_fixing;
	*nr_threads = scrub_nproc(ctx) + 1;
	*rshift = 0;
	return 0;
}
