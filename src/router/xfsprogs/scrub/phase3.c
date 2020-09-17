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
#include "counter.h"
#include "inodes.h"
#include "progress.h"
#include "scrub.h"
#include "repair.h"

/* Phase 3: Scan all inodes. */

/*
 * Run a per-file metadata scanner.  We use the ino/gen interface to
 * ensure that the inode we're checking matches what the inode scan
 * told us to look at.
 */
static int
scrub_fd(
	struct scrub_ctx	*ctx,
	int			(*fn)(struct scrub_ctx *ctx, uint64_t ino,
				      uint32_t gen, struct action_list *a),
	struct xfs_bulkstat	*bs,
	struct action_list	*alist)
{
	return fn(ctx, bs->bs_ino, bs->bs_gen, alist);
}

struct scrub_inode_ctx {
	struct ptcounter	*icount;
	bool			aborted;
};

/* Report a filesystem error that the vfs fed us on close. */
static void
report_close_error(
	struct scrub_ctx	*ctx,
	struct xfs_bulkstat	*bstat)
{
	char			descr[DESCR_BUFSZ];
	int			old_errno = errno;

	scrub_render_ino_descr(ctx, descr, DESCR_BUFSZ, bstat->bs_ino,
			bstat->bs_gen, NULL);
	errno = old_errno;
	str_errno(ctx, descr);
}

/* Verify the contents, xattrs, and extent maps of an inode. */
static int
scrub_inode(
	struct scrub_ctx	*ctx,
	struct xfs_handle	*handle,
	struct xfs_bulkstat	*bstat,
	void			*arg)
{
	struct action_list	alist;
	struct scrub_inode_ctx	*ictx = arg;
	struct ptcounter	*icount = ictx->icount;
	xfs_agnumber_t		agno;
	int			fd = -1;
	int			error;

	action_list_init(&alist);
	agno = cvt_ino_to_agno(&ctx->mnt, bstat->bs_ino);
	background_sleep();

	/* Try to open the inode to pin it. */
	if (S_ISREG(bstat->bs_mode)) {
		fd = scrub_open_handle(handle);
		/* Stale inode means we scan the whole cluster again. */
		if (fd < 0 && errno == ESTALE)
			return ESTALE;
	}

	/* Scrub the inode. */
	error = scrub_fd(ctx, scrub_inode_fields, bstat, &alist);
	if (error)
		goto out;

	error = action_list_process_or_defer(ctx, agno, &alist);
	if (error)
		goto out;

	/* Scrub all block mappings. */
	error = scrub_fd(ctx, scrub_data_fork, bstat, &alist);
	if (error)
		goto out;
	error = scrub_fd(ctx, scrub_attr_fork, bstat, &alist);
	if (error)
		goto out;
	error = scrub_fd(ctx, scrub_cow_fork, bstat, &alist);
	if (error)
		goto out;

	error = action_list_process_or_defer(ctx, agno, &alist);
	if (error)
		goto out;

	if (S_ISLNK(bstat->bs_mode)) {
		/* Check symlink contents. */
		error = scrub_symlink(ctx, bstat->bs_ino, bstat->bs_gen,
				&alist);
	} else if (S_ISDIR(bstat->bs_mode)) {
		/* Check the directory entries. */
		error = scrub_fd(ctx, scrub_dir, bstat, &alist);
	}
	if (error)
		goto out;

	/* Check all the extended attributes. */
	error = scrub_fd(ctx, scrub_attr, bstat, &alist);
	if (error)
		goto out;

	/* Check parent pointers. */
	error = scrub_fd(ctx, scrub_parent, bstat, &alist);
	if (error)
		goto out;

	/* Try to repair the file while it's open. */
	error = action_list_process_or_defer(ctx, agno, &alist);
	if (error)
		goto out;

out:
	if (error)
		ictx->aborted = true;

	error = ptcounter_add(icount, 1);
	if (error) {
		str_liberror(ctx, error,
				_("incrementing scanned inode counter"));
		ictx->aborted = true;
	}
	progress_add(1);
	action_list_defer(ctx, agno, &alist);
	if (fd >= 0) {
		int	err2;

		err2 = close(fd);
		if (err2) {
			report_close_error(ctx, bstat);
			ictx->aborted = true;
		}
	}

	if (!error && ictx->aborted)
		error = ECANCELED;
	return error;
}

/* Verify all the inodes in a filesystem. */
int
phase3_func(
	struct scrub_ctx	*ctx)
{
	struct scrub_inode_ctx	ictx = { NULL };
	uint64_t		val;
	int			err;

	err = ptcounter_alloc(scrub_nproc(ctx), &ictx.icount);
	if (err) {
		str_liberror(ctx, err, _("creating scanned inode counter"));
		return err;
	}

	err = scrub_scan_all_inodes(ctx, scrub_inode, &ictx);
	if (!err && ictx.aborted)
		err = ECANCELED;
	if (err)
		goto free;

	scrub_report_preen_triggers(ctx);
	err = ptcounter_value(ictx.icount, &val);
	if (err) {
		str_liberror(ctx, err, _("summing scanned inode counter"));
		return err;
	}

	ctx->inodes_checked = val;
free:
	ptcounter_free(ictx.icount);
	return err;
}

/* Estimate how much work we're going to do. */
int
phase3_estimate(
	struct scrub_ctx	*ctx,
	uint64_t		*items,
	unsigned int		*nr_threads,
	int			*rshift)
{
	*items = ctx->mnt_sv.f_files - ctx->mnt_sv.f_ffree;
	*nr_threads = scrub_nproc(ctx);
	*rshift = 0;
	return 0;
}
