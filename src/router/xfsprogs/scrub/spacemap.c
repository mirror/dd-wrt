// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include <linux/fsmap.h>
#include "libfrog/workqueue.h"
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "spacemap.h"

/*
 * Filesystem space map iterators.
 *
 * Logically, we call GETFSMAP to fetch a set of space map records and
 * call a function to iterate over the records.  However, that's not
 * what actually happens -- the work is split into separate items, with
 * each AG, the realtime device, and the log device getting their own
 * work items.  For an XFS with a realtime device and an external log,
 * this means that we can have up to ($agcount + 2) threads running at
 * once.
 *
 * This comes into play if we want to have per-workitem memory.  Maybe.
 * XXX: do we really need all that ?
 */

#define FSMAP_NR	65536

/*
 * Iterate all the fs block mappings between the two keys.  Returns 0 or a
 * positive error number.
 */
int
scrub_iterate_fsmap(
	struct scrub_ctx	*ctx,
	struct fsmap		*keys,
	scrub_fsmap_iter_fn	fn,
	void			*arg)
{
	struct fsmap_head	*head;
	struct fsmap		*p;
	int			i;
	int			error;

	head = calloc(1, fsmap_sizeof(FSMAP_NR));
	if (!head)
		return errno;

	memcpy(head->fmh_keys, keys, sizeof(struct fsmap) * 2);
	head->fmh_count = FSMAP_NR;

	while ((error = ioctl(ctx->mnt.fd, FS_IOC_GETFSMAP, head)) == 0) {
		for (i = 0, p = head->fmh_recs;
		     i < head->fmh_entries;
		     i++, p++) {
			error = fn(ctx, p, arg);
			if (error)
				goto out;
			if (scrub_excessive_errors(ctx))
				goto out;
		}

		if (head->fmh_entries == 0)
			break;
		p = &head->fmh_recs[head->fmh_entries - 1];
		if (p->fmr_flags & FMR_OF_LAST)
			break;
		fsmap_advance(head);
	}
	if (error)
		error = errno;
out:
	free(head);
	return error;
}

/* GETFSMAP wrappers routines. */
struct scan_blocks {
	scrub_fsmap_iter_fn	fn;
	void			*arg;
	bool			aborted;
};

/* Iterate all the reverse mappings of an AG. */
static void
scan_ag_rmaps(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;
	struct scan_blocks	*sbx = arg;
	struct fsmap		keys[2];
	off64_t			bperag;
	int			ret;

	bperag = (off64_t)ctx->mnt.fsgeom.agblocks *
		 (off64_t)ctx->mnt.fsgeom.blocksize;

	memset(keys, 0, sizeof(struct fsmap) * 2);
	keys->fmr_device = ctx->fsinfo.fs_datadev;
	keys->fmr_physical = agno * bperag;
	(keys + 1)->fmr_device = ctx->fsinfo.fs_datadev;
	(keys + 1)->fmr_physical = ((agno + 1) * bperag) - 1;
	(keys + 1)->fmr_owner = ULLONG_MAX;
	(keys + 1)->fmr_offset = ULLONG_MAX;
	(keys + 1)->fmr_flags = UINT_MAX;

	if (sbx->aborted)
		return;

	ret = scrub_iterate_fsmap(ctx, keys, sbx->fn, sbx->arg);
	if (ret) {
		char		descr[DESCR_BUFSZ];

		snprintf(descr, DESCR_BUFSZ, _("dev %d:%d AG %u fsmap"),
					major(ctx->fsinfo.fs_datadev),
					minor(ctx->fsinfo.fs_datadev),
					agno);
		str_liberror(ctx, ret, descr);
		sbx->aborted = true;
	}
}

/* Iterate all the reverse mappings of a standalone device. */
static void
scan_dev_rmaps(
	struct scrub_ctx	*ctx,
	int			idx,
	dev_t			dev,
	struct scan_blocks	*sbx)
{
	struct fsmap		keys[2];
	int			ret;

	memset(keys, 0, sizeof(struct fsmap) * 2);
	keys->fmr_device = dev;
	(keys + 1)->fmr_device = dev;
	(keys + 1)->fmr_physical = ULLONG_MAX;
	(keys + 1)->fmr_owner = ULLONG_MAX;
	(keys + 1)->fmr_offset = ULLONG_MAX;
	(keys + 1)->fmr_flags = UINT_MAX;

	if (sbx->aborted)
		return;

	ret = scrub_iterate_fsmap(ctx, keys, sbx->fn, sbx->arg);
	if (ret) {
		char		descr[DESCR_BUFSZ];

		snprintf(descr, DESCR_BUFSZ, _("dev %d:%d fsmap"),
				major(dev), minor(dev));
		str_liberror(ctx, ret, descr);
		sbx->aborted = true;
	}
}

/* Iterate all the reverse mappings of the realtime device. */
static void
scan_rt_rmaps(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;

	scan_dev_rmaps(ctx, agno, ctx->fsinfo.fs_rtdev, arg);
}

/* Iterate all the reverse mappings of the log device. */
static void
scan_log_rmaps(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;

	scan_dev_rmaps(ctx, agno, ctx->fsinfo.fs_logdev, arg);
}

/*
 * Scan all the blocks in a filesystem.  If errors occur, this function will
 * log them and return nonzero.
 */
int
scrub_scan_all_spacemaps(
	struct scrub_ctx	*ctx,
	scrub_fsmap_iter_fn	fn,
	void			*arg)
{
	struct workqueue	wq;
	struct scan_blocks	sbx = {
		.fn		= fn,
		.arg		= arg,
	};
	xfs_agnumber_t		agno;
	int			ret;

	ret = -workqueue_create(&wq, (struct xfs_mount *)ctx,
			scrub_nproc_workqueue(ctx));
	if (ret) {
		str_liberror(ctx, ret, _("creating fsmap workqueue"));
		return ret;
	}
	if (ctx->fsinfo.fs_rt) {
		ret = -workqueue_add(&wq, scan_rt_rmaps,
				ctx->mnt.fsgeom.agcount + 1, &sbx);
		if (ret) {
			sbx.aborted = true;
			str_liberror(ctx, ret, _("queueing rtdev fsmap work"));
			goto out;
		}
	}
	if (ctx->fsinfo.fs_log) {
		ret = -workqueue_add(&wq, scan_log_rmaps,
				ctx->mnt.fsgeom.agcount + 2, &sbx);
		if (ret) {
			sbx.aborted = true;
			str_liberror(ctx, ret, _("queueing logdev fsmap work"));
			goto out;
		}
	}
	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++) {
		ret = -workqueue_add(&wq, scan_ag_rmaps, agno, &sbx);
		if (ret) {
			sbx.aborted = true;
			str_liberror(ctx, ret, _("queueing per-AG fsmap work"));
			break;
		}
	}
out:
	ret = -workqueue_terminate(&wq);
	if (ret) {
		sbx.aborted = true;
		str_liberror(ctx, ret, _("finishing fsmap work"));
	}
	workqueue_destroy(&wq);

	if (!ret && sbx.aborted)
		ret = -1;

	return ret;
}
