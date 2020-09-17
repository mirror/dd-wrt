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
	size_t				unfixed;
	size_t				new_unfixed;
	unsigned int			flags = 0;
	int				ret;

	alist = &ctx->action_lists[agno];
	unfixed = action_list_length(alist);

	/* Repair anything broken until we fail to make progress. */
	do {
		ret = action_list_process(ctx, ctx->mnt.fd, alist, flags);
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
	ret = action_list_process(ctx, ctx->mnt.fd, alist, flags);
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

	pthread_mutex_lock(&ctx->lock);
	if (ctx->corruptions_found == 0 && ctx->unfixable_errors == 0 &&
	    want_fstrim) {
		fstrim(ctx);
		progress_add(1);
	}
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/* Fix everything that needs fixing. */
int
phase4_func(
	struct scrub_ctx	*ctx)
{
	int			ret;

	/*
	 * Check the summary counters early.  Normally we do this during phase
	 * seven, but some of the cross-referencing requires fairly-accurate
	 * counters, so counter repairs have to be put on the list now so that
	 * they get fixed before we stop retrying unfixed metadata repairs.
	 */
	ret = scrub_fs_summary(ctx, &ctx->action_lists[0]);
	if (ret)
		return ret;

	return repair_everything(ctx);
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
	size_t			need_fixing = 0;

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++)
		need_fixing += action_list_length(&ctx->action_lists[agno]);
	need_fixing++;
	*items = need_fixing;
	*nr_threads = scrub_nproc(ctx) + 1;
	*rshift = 0;
	return 0;
}
