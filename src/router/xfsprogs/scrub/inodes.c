// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/statvfs.h>
#include "platform_defs.h"
#include "xfs_arch.h"
#include "xfs_format.h"
#include "handle.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "inodes.h"
#include "libfrog/fsgeom.h"
#include "libfrog/bulkstat.h"

/*
 * Iterate a range of inodes.
 *
 * This is a little more involved than repeatedly asking BULKSTAT for a
 * buffer's worth of stat data for some number of inodes.  We want to scan as
 * many of the inodes that the inobt thinks there are, including the ones that
 * are broken, but if we ask for n inodes starting at x, it'll skip the bad
 * ones and fill from beyond the range (x + n).
 *
 * Therefore, we ask INUMBERS to return one inobt chunk's worth of inode
 * bitmap information.  Then we try to BULKSTAT only the inodes that were
 * present in that chunk, and compare what we got against what INUMBERS said
 * was there.  If there's a mismatch, we know that we have an inode that fails
 * the verifiers but we can inject the bulkstat information to force the scrub
 * code to deal with the broken inodes.
 *
 * If the iteration function returns ESTALE, that means that the inode has
 * been deleted and possibly recreated since the BULKSTAT call.  We wil
 * refresh the stat information and try again up to 30 times before reporting
 * the staleness as an error.
 */

/*
 * Run bulkstat on an entire inode allocation group, then check that we got
 * exactly the inodes we expected.  If not, load them one at a time (or fake
 * it) into the bulkstat data.
 */
static void
bulkstat_for_inumbers(
	struct scrub_ctx	*ctx,
	const char		*descr,
	const struct xfs_inumbers *inumbers,
	struct xfs_bulkstat_req	*breq)
{
	struct xfs_bulkstat	*bstat = breq->bulkstat;
	struct xfs_bulkstat	*bs;
	int			i;
	int			error;

	/* First we try regular bulkstat, for speed. */
	breq->hdr.ino = inumbers->xi_startino;
	breq->hdr.icount = inumbers->xi_alloccount;
	error = -xfrog_bulkstat(&ctx->mnt, breq);
	if (error) {
		char	errbuf[DESCR_BUFSZ];

		str_info(ctx, descr, "%s",
			 strerror_r(error, errbuf, DESCR_BUFSZ));
	}

	/*
	 * Check each of the stats we got back to make sure we got the inodes
	 * we asked for.
	 */
	for (i = 0, bs = bstat; i < XFS_INODES_PER_CHUNK; i++) {
		if (!(inumbers->xi_allocmask & (1ULL << i)))
			continue;
		if (bs->bs_ino == inumbers->xi_startino + i) {
			bs++;
			continue;
		}

		/* Load the one inode. */
		error = -xfrog_bulkstat_single(&ctx->mnt,
				inumbers->xi_startino + i, 0, bs);
		if (error || bs->bs_ino != inumbers->xi_startino + i) {
			memset(bs, 0, sizeof(struct xfs_bulkstat));
			bs->bs_ino = inumbers->xi_startino + i;
			bs->bs_blksize = ctx->mnt_sv.f_frsize;
		}
		bs++;
	}
}

/* BULKSTAT wrapper routines. */
struct scan_inodes {
	scrub_inode_iter_fn	fn;
	void			*arg;
	bool			aborted;
};

/*
 * Call into the filesystem for inode/bulkstat information and call our
 * iterator function.  We'll try to fill the bulkstat information in batches,
 * but we also can detect iget failures.
 */
static void
scan_ag_inodes(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct xfs_handle	handle;
	char			descr[DESCR_BUFSZ];
	struct xfs_inumbers_req	*ireq;
	struct xfs_bulkstat_req	*breq;
	struct scan_inodes	*si = arg;
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;
	struct xfs_bulkstat	*bs;
	struct xfs_inumbers	*inumbers;
	int			i;
	int			error;
	int			stale_count = 0;

	snprintf(descr, DESCR_BUFSZ, _("dev %d:%d AG %u inodes"),
				major(ctx->fsinfo.fs_datadev),
				minor(ctx->fsinfo.fs_datadev),
				agno);

	memcpy(&handle.ha_fsid, ctx->fshandle, sizeof(handle.ha_fsid));
	handle.ha_fid.fid_len = sizeof(xfs_fid_t) -
			sizeof(handle.ha_fid.fid_len);
	handle.ha_fid.fid_pad = 0;

	error = -xfrog_bulkstat_alloc_req(XFS_INODES_PER_CHUNK, 0, &breq);
	if (error) {
		str_liberror(ctx, error, descr);
		si->aborted = true;
		return;
	}

	error = -xfrog_inumbers_alloc_req(1, 0, &ireq);
	if (error) {
		str_liberror(ctx, error, descr);
		free(breq);
		si->aborted = true;
		return;
	}
	inumbers = &ireq->inumbers[0];
	xfrog_inumbers_set_ag(ireq, agno);

	/* Find the inode chunk & alloc mask */
	error = -xfrog_inumbers(&ctx->mnt, ireq);
	while (!error && !si->aborted && ireq->hdr.ocount > 0) {
		/*
		 * We can have totally empty inode chunks on filesystems where
		 * there are more than 64 inodes per block.  Skip these.
		 */
		if (inumbers->xi_alloccount == 0)
			goto igrp_retry;

		bulkstat_for_inumbers(ctx, descr, inumbers, breq);

		/* Iterate all the inodes. */
		for (i = 0, bs = breq->bulkstat;
		     !si->aborted && i < inumbers->xi_alloccount;
		     i++, bs++) {
			handle.ha_fid.fid_ino = bs->bs_ino;
			handle.ha_fid.fid_gen = bs->bs_gen;
			error = si->fn(ctx, &handle, bs, si->arg);
			switch (error) {
			case 0:
				break;
			case ESTALE: {
				char	idescr[DESCR_BUFSZ];

				stale_count++;
				if (stale_count < 30) {
					ireq->hdr.ino = inumbers->xi_startino;
					goto igrp_retry;
				}
				scrub_render_ino_descr(ctx, idescr, DESCR_BUFSZ,
						bs->bs_ino, bs->bs_gen, NULL);
				str_info(ctx, idescr,
_("Changed too many times during scan; giving up."));
				break;
			}
			case ECANCELED:
				error = 0;
				/* fall thru */
			default:
				goto err;
			}
			if (scrub_excessive_errors(ctx)) {
				si->aborted = true;
				goto out;
			}
		}

		stale_count = 0;
igrp_retry:
		error = -xfrog_inumbers(&ctx->mnt, ireq);
	}

err:
	if (error) {
		str_liberror(ctx, error, descr);
		si->aborted = true;
	}
out:
	free(ireq);
	free(breq);
}

/*
 * Scan all the inodes in a filesystem.  On error, this function will log
 * an error message and return -1.
 */
int
scrub_scan_all_inodes(
	struct scrub_ctx	*ctx,
	scrub_inode_iter_fn	fn,
	void			*arg)
{
	struct scan_inodes	si = {
		.fn		= fn,
		.arg		= arg,
	};
	xfs_agnumber_t		agno;
	struct workqueue	wq;
	int			ret;

	ret = -workqueue_create(&wq, (struct xfs_mount *)ctx,
			scrub_nproc_workqueue(ctx));
	if (ret) {
		str_liberror(ctx, ret, _("creating bulkstat workqueue"));
		return -1;
	}

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++) {
		ret = -workqueue_add(&wq, scan_ag_inodes, agno, &si);
		if (ret) {
			si.aborted = true;
			str_liberror(ctx, ret, _("queueing bulkstat work"));
			break;
		}
	}

	ret = -workqueue_terminate(&wq);
	if (ret) {
		si.aborted = true;
		str_liberror(ctx, ret, _("finishing bulkstat work"));
	}
	workqueue_destroy(&wq);

	return si.aborted ? -1 : 0;
}

/* Open a file by handle, returning either the fd or -1 on error. */
int
scrub_open_handle(
	struct xfs_handle	*handle)
{
	return open_by_fshandle(handle, sizeof(*handle),
			O_RDONLY | O_NOATIME | O_NOFOLLOW | O_NOCTTY);
}
