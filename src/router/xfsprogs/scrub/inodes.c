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
#include "handle.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "inodes.h"
#include "descr.h"
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
	struct descr		*dsc,
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

		str_info(ctx, descr_render(dsc), "%s",
			 strerror_r(error, errbuf, DESCR_BUFSZ));
	}

	/*
	 * Check each of the stats we got back to make sure we got the inodes
	 * we asked for.
	 */
	for (i = 0, bs = bstat; i < LIBFROG_BULKSTAT_CHUNKSIZE; i++) {
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
	struct workqueue	wq_bulkstat;
	scrub_inode_iter_fn	fn;
	void			*arg;
	unsigned int		nr_threads;
	bool			aborted;
};

/*
 * A single unit of inode scan work.  This contains a pointer to the parent
 * information, followed by an INUMBERS request structure, followed by a
 * BULKSTAT request structure.  The last two are VLAs, so we can't represent
 * them here.
 */
struct scan_ichunk {
	struct scan_inodes	*si;
};

static inline struct xfs_inumbers_req *
ichunk_to_inumbers(
	struct scan_ichunk	*ichunk)
{
	char			*p = (char *)ichunk;

	return (struct xfs_inumbers_req *)(p + sizeof(struct scan_ichunk));
}

static inline struct xfs_bulkstat_req *
ichunk_to_bulkstat(
	struct scan_ichunk	*ichunk)
{
	char			*p = (char *)ichunk_to_inumbers(ichunk);

	return (struct xfs_bulkstat_req *)(p + XFS_INUMBERS_REQ_SIZE(1));
}

static inline int
alloc_ichunk(
	struct scan_inodes	*si,
	uint32_t		agno,
	uint64_t		startino,
	struct scan_ichunk	**ichunkp)
{
	struct scan_ichunk	*ichunk;
	struct xfs_inumbers_req	*ireq;
	struct xfs_bulkstat_req	*breq;

	ichunk = calloc(1, sizeof(struct scan_ichunk) +
			   XFS_INUMBERS_REQ_SIZE(1) +
			   XFS_BULKSTAT_REQ_SIZE(LIBFROG_BULKSTAT_CHUNKSIZE));
	if (!ichunk)
		return -errno;

	ichunk->si = si;

	ireq = ichunk_to_inumbers(ichunk);
	ireq->hdr.icount = 1;
	ireq->hdr.ino = startino;
	ireq->hdr.agno = agno;
	ireq->hdr.flags |= XFS_BULK_IREQ_AGNO;

	breq = ichunk_to_bulkstat(ichunk);
	breq->hdr.icount = LIBFROG_BULKSTAT_CHUNKSIZE;

	*ichunkp = ichunk;
	return 0;
}

static int
render_ino_from_bulkstat(
	struct scrub_ctx	*ctx,
	char			*buf,
	size_t			buflen,
	void			*data)
{
	struct xfs_bulkstat	*bstat = data;

	return scrub_render_ino_descr(ctx, buf, buflen, bstat->bs_ino,
			bstat->bs_gen, NULL);
}

static int
render_inumbers_from_agno(
	struct scrub_ctx	*ctx,
	char			*buf,
	size_t			buflen,
	void			*data)
{
	xfs_agnumber_t		*agno = data;

	return snprintf(buf, buflen, _("dev %d:%d AG %u inodes"),
				major(ctx->fsinfo.fs_datadev),
				minor(ctx->fsinfo.fs_datadev),
				*agno);
}

/*
 * Call BULKSTAT for information on a single chunk's worth of inodes and call
 * our iterator function.  We'll try to fill the bulkstat information in
 * batches, but we also can detect iget failures.
 */
static void
scan_ag_bulkstat(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct xfs_handle	handle = { };
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;
	struct scan_ichunk	*ichunk = arg;
	struct xfs_inumbers_req	*ireq = ichunk_to_inumbers(ichunk);
	struct xfs_bulkstat_req	*breq = ichunk_to_bulkstat(ichunk);
	struct scan_inodes	*si = ichunk->si;
	struct xfs_bulkstat	*bs;
	struct xfs_inumbers	*inumbers = &ireq->inumbers[0];
	uint64_t		last_ino = 0;
	int			i;
	int			error;
	int			stale_count = 0;
	DEFINE_DESCR(dsc_bulkstat, ctx, render_ino_from_bulkstat);
	DEFINE_DESCR(dsc_inumbers, ctx, render_inumbers_from_agno);

	descr_set(&dsc_inumbers, &agno);

	memcpy(&handle.ha_fsid, ctx->fshandle, sizeof(handle.ha_fsid));
	handle.ha_fid.fid_len = sizeof(xfs_fid_t) -
			sizeof(handle.ha_fid.fid_len);
	handle.ha_fid.fid_pad = 0;

retry:
	bulkstat_for_inumbers(ctx, &dsc_inumbers, inumbers, breq);

	/* Iterate all the inodes. */
	bs = &breq->bulkstat[0];
	for (i = 0; !si->aborted && i < inumbers->xi_alloccount; i++, bs++) {
		uint64_t	scan_ino = bs->bs_ino;

		/* ensure forward progress if we retried */
		if (scan_ino < last_ino)
			continue;

		descr_set(&dsc_bulkstat, bs);
		handle.ha_fid.fid_ino = scan_ino;
		handle.ha_fid.fid_gen = bs->bs_gen;
		error = si->fn(ctx, &handle, bs, si->arg);
		switch (error) {
		case 0:
			break;
		case ESTALE: {
			stale_count++;
			if (stale_count < 30) {
				ireq->hdr.ino = inumbers->xi_startino;
				error = -xfrog_inumbers(&ctx->mnt, ireq);
				if (error)
					goto err;
				goto retry;
			}
			str_info(ctx, descr_render(&dsc_bulkstat),
_("Changed too many times during scan; giving up."));
			si->aborted = true;
			goto out;
		}
		case ECANCELED:
			error = 0;
			fallthrough;
		default:
			goto err;
		}
		if (scrub_excessive_errors(ctx)) {
			si->aborted = true;
			goto out;
		}
		last_ino = scan_ino;
	}

err:
	if (error) {
		str_liberror(ctx, error, descr_render(&dsc_bulkstat));
		si->aborted = true;
	}
out:
	free(ichunk);
}

/*
 * Call INUMBERS for information about inode chunks, then queue the inumbers
 * responses in the bulkstat workqueue.  This helps us maximize CPU parallelism
 * if the filesystem AGs are not evenly loaded.
 */
static void
scan_ag_inumbers(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct scan_ichunk	*ichunk = NULL;
	struct scan_inodes	*si = arg;
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;
	struct xfs_inumbers_req	*ireq;
	uint64_t		nextino = cvt_agino_to_ino(&ctx->mnt, agno, 0);
	int			error;
	DEFINE_DESCR(dsc, ctx, render_inumbers_from_agno);

	descr_set(&dsc, &agno);

	error = alloc_ichunk(si, agno, 0, &ichunk);
	if (error)
		goto err;
	ireq = ichunk_to_inumbers(ichunk);

	/* Find the inode chunk & alloc mask */
	error = -xfrog_inumbers(&ctx->mnt, ireq);
	while (!error && !si->aborted && ireq->hdr.ocount > 0) {
		/*
		 * Make sure that we always make forward progress while we
		 * scan the inode btree.
		 */
		if (nextino > ireq->inumbers[0].xi_startino) {
			str_corrupt(ctx, descr_render(&dsc),
	_("AG %u inode btree is corrupt near agino %lu, got %lu"), agno,
				cvt_ino_to_agino(&ctx->mnt, nextino),
				cvt_ino_to_agino(&ctx->mnt,
						ireq->inumbers[0].xi_startino));
			si->aborted = true;
			break;
		}
		nextino = ireq->hdr.ino;

		if (ireq->inumbers[0].xi_alloccount == 0) {
			/*
			 * We can have totally empty inode chunks on
			 * filesystems where there are more than 64 inodes per
			 * block.  Skip these.
			 */
			;
		} else if (si->nr_threads > 0) {
			/* Queue this inode chunk on the bulkstat workqueue. */
			error = -workqueue_add(&si->wq_bulkstat,
					scan_ag_bulkstat, agno, ichunk);
			if (error) {
				si->aborted = true;
				str_liberror(ctx, error,
						_("queueing bulkstat work"));
				goto out;
			}
			ichunk = NULL;
		} else {
			/*
			 * Only one thread, call bulkstat directly.  Remember,
			 * ichunk is freed by the worker before returning.
			 */
			scan_ag_bulkstat(wq, agno, ichunk);
			ichunk = NULL;
			if (si->aborted)
				break;
		}

		if (!ichunk) {
			error = alloc_ichunk(si, agno, nextino, &ichunk);
			if (error)
				goto err;
		}
		ireq = ichunk_to_inumbers(ichunk);

		error = -xfrog_inumbers(&ctx->mnt, ireq);
	}

err:
	if (error) {
		str_liberror(ctx, error, descr_render(&dsc));
		si->aborted = true;
	}
out:
	if (ichunk)
		free(ichunk);
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
		.nr_threads	= scrub_nproc_workqueue(ctx),
	};
	xfs_agnumber_t		agno;
	struct workqueue	wq_inumbers;
	unsigned int		max_bulkstat;
	int			ret;

	/*
	 * The bulkstat workqueue should queue at most one inobt block's worth
	 * of inode chunk records per worker thread.  If we're running in
	 * single thread mode (nr_threads==0) then we skip the workqueues.
	 */
	max_bulkstat = si.nr_threads * (ctx->mnt.fsgeom.blocksize / 16);

	ret = -workqueue_create_bound(&si.wq_bulkstat, (struct xfs_mount *)ctx,
			si.nr_threads, max_bulkstat);
	if (ret) {
		str_liberror(ctx, ret, _("creating bulkstat workqueue"));
		return -1;
	}

	ret = -workqueue_create(&wq_inumbers, (struct xfs_mount *)ctx,
			si.nr_threads);
	if (ret) {
		str_liberror(ctx, ret, _("creating inumbers workqueue"));
		si.aborted = true;
		goto kill_bulkstat;
	}

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++) {
		ret = -workqueue_add(&wq_inumbers, scan_ag_inumbers, agno, &si);
		if (ret) {
			si.aborted = true;
			str_liberror(ctx, ret, _("queueing inumbers work"));
			break;
		}
	}

	ret = -workqueue_terminate(&wq_inumbers);
	if (ret) {
		si.aborted = true;
		str_liberror(ctx, ret, _("finishing inumbers work"));
	}
	workqueue_destroy(&wq_inumbers);

kill_bulkstat:
	ret = -workqueue_terminate(&si.wq_bulkstat);
	if (ret) {
		si.aborted = true;
		str_liberror(ctx, ret, _("finishing bulkstat work"));
	}
	workqueue_destroy(&si.wq_bulkstat);

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
