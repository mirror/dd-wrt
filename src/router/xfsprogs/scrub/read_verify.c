// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include "libfrog/ptvar.h"
#include "libfrog/workqueue.h"
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "counter.h"
#include "disk.h"
#include "read_verify.h"
#include "progress.h"

/*
 * Read Verify Pool
 *
 * Manages the data block read verification phase.  The caller schedules
 * verification requests, which are then scheduled to be run by a thread
 * pool worker.  Adjacent (or nearly adjacent) requests can be combined
 * to reduce overhead when free space fragmentation is high.  The thread
 * pool takes care of issuing multiple IOs to the device, if possible.
 */

/*
 * Perform all IO in 32M chunks.  This cannot exceed 65536 sectors
 * because that's the biggest SCSI VERIFY(16) we dare to send.
 */
#define RVP_IO_MAX_SIZE		(33554432)

/*
 * If we're running in the background then we perform IO in 128k chunks
 * to reduce the load on the IO subsystem.
 */
#define RVP_BACKGROUND_IO_MAX_SIZE	(131072)

/* What's the real maximum IO size? */
static inline unsigned int
rvp_io_max_size(void)
{
	return bg_mode > 0 ? RVP_BACKGROUND_IO_MAX_SIZE : RVP_IO_MAX_SIZE;
}

/* Tolerate 64k holes in adjacent read verify requests. */
#define RVP_IO_BATCH_LOCALITY	(65536)

struct read_verify {
	void			*io_end_arg;
	struct disk		*io_disk;
	uint64_t		io_start;	/* bytes */
	uint64_t		io_length;	/* bytes */
};

struct read_verify_pool {
	struct workqueue	wq;		/* thread pool */
	struct scrub_ctx	*ctx;		/* scrub context */
	void			*readbuf;	/* read buffer */
	struct ptcounter	*verified_bytes;
	struct ptvar		*rvstate;	/* combines read requests */
	struct disk		*disk;		/* which disk? */
	read_verify_ioerr_fn_t	ioerr_fn;	/* io error callback */
	size_t			miniosz;	/* minimum io size, bytes */

	/*
	 * Store a runtime error code here so that we can stop the pool and
	 * return it to the caller.
	 */
	int			runtime_error;
};

/*
 * Create a thread pool to run read verifiers.
 *
 * @disk is the disk we want to verify.
 * @miniosz is the minimum size of an IO to expect (in bytes).
 * @ioerr_fn will be called when IO errors occur.
 * @submitter_threads is the number of threads that may be sending verify
 * requests at any given time.
 */
int
read_verify_pool_alloc(
	struct scrub_ctx		*ctx,
	struct disk			*disk,
	size_t				miniosz,
	read_verify_ioerr_fn_t		ioerr_fn,
	unsigned int			submitter_threads,
	struct read_verify_pool		**prvp)
{
	struct read_verify_pool		*rvp;
	unsigned int			verifier_threads = disk_heads(disk);
	int				ret;

	/*
	 * The minimum IO size must be a multiple of the disk sector size
	 * and a factor of the max io size.
	 */
	if (miniosz % disk->d_lbasize)
		return EINVAL;
	if (rvp_io_max_size() % miniosz)
		return EINVAL;

	rvp = calloc(1, sizeof(struct read_verify_pool));
	if (!rvp)
		return errno;

	ret = posix_memalign((void **)&rvp->readbuf, page_size,
			rvp_io_max_size());
	if (ret)
		goto out_free;
	ret = ptcounter_alloc(verifier_threads, &rvp->verified_bytes);
	if (ret)
		goto out_buf;
	rvp->miniosz = miniosz;
	rvp->ctx = ctx;
	rvp->disk = disk;
	rvp->ioerr_fn = ioerr_fn;
	ret = -ptvar_alloc(submitter_threads, sizeof(struct read_verify),
			&rvp->rvstate);
	if (ret)
		goto out_counter;
	ret = -workqueue_create(&rvp->wq, (struct xfs_mount *)rvp,
			verifier_threads == 1 ? 0 : verifier_threads);
	if (ret)
		goto out_rvstate;
	*prvp = rvp;
	return 0;

out_rvstate:
	ptvar_free(rvp->rvstate);
out_counter:
	ptcounter_free(rvp->verified_bytes);
out_buf:
	free(rvp->readbuf);
out_free:
	free(rvp);
	return ret;
}

/* Abort all verification work. */
void
read_verify_pool_abort(
	struct read_verify_pool		*rvp)
{
	if (!rvp->runtime_error)
		rvp->runtime_error = ECANCELED;
	workqueue_terminate(&rvp->wq);
}

/* Finish up any read verification work. */
int
read_verify_pool_flush(
	struct read_verify_pool		*rvp)
{
	return -workqueue_terminate(&rvp->wq);
}

/* Finish up any read verification work and tear it down. */
void
read_verify_pool_destroy(
	struct read_verify_pool		*rvp)
{
	workqueue_destroy(&rvp->wq);
	ptvar_free(rvp->rvstate);
	ptcounter_free(rvp->verified_bytes);
	free(rvp->readbuf);
	free(rvp);
}

/*
 * Issue a read-verify IO in big batches.
 */
static void
read_verify(
	struct workqueue		*wq,
	xfs_agnumber_t			agno,
	void				*arg)
{
	struct read_verify		*rv = arg;
	struct read_verify_pool		*rvp;
	unsigned long long		verified = 0;
	ssize_t				io_max_size;
	ssize_t				sz;
	ssize_t				len;
	int				read_error;
	int				ret;

	rvp = (struct read_verify_pool *)wq->wq_ctx;
	if (rvp->runtime_error)
		return;

	io_max_size = rvp_io_max_size();

	while (rv->io_length > 0) {
		read_error = 0;
		len = min(rv->io_length, io_max_size);
		dbg_printf("diskverify %d %"PRIu64" %zu\n", rvp->disk->d_fd,
				rv->io_start, len);
		sz = disk_read_verify(rvp->disk, rvp->readbuf, rv->io_start,
				len);
		if (sz == len && io_max_size < rvp->miniosz) {
			/*
			 * If the verify request was 100% successful and less
			 * than a single block in length, we were trying to
			 * read to the end of a block after a short read.  That
			 * suggests there's something funny with this device,
			 * so single-step our way through the rest of the @rv
			 * range.
			 */
			io_max_size = rvp->miniosz;
		} else if (sz < 0) {
			read_error = errno;

			/* Runtime error, bail out... */
			if (read_error != EIO && read_error != EILSEQ) {
				rvp->runtime_error = read_error;
				return;
			}

			/*
			 * A direct read encountered an error while performing
			 * a multi-block read.  Reduce the transfer size to a
			 * single block so that we can identify the exact range
			 * of bad blocks and good blocks.  We single-step all
			 * the way to the end of the @rv range, (re)starting
			 * with the block that just failed.
			 */
			if (io_max_size > rvp->miniosz) {
				io_max_size = rvp->miniosz;
				continue;
			}

			/*
			 * A direct read hit an error while we were stepping
			 * through single blocks.  Mark everything bad from
			 * io_start to the next miniosz block.
			 */
			sz = rvp->miniosz - (rv->io_start % rvp->miniosz);
			dbg_printf("IOERR %d @ %"PRIu64" %zu err %d\n",
					rvp->disk->d_fd, rv->io_start, sz,
					read_error);
			rvp->ioerr_fn(rvp->ctx, rvp->disk, rv->io_start, sz,
					read_error, rv->io_end_arg);
		} else if (sz < len) {
			/*
			 * A short direct read suggests that we might have hit
			 * an IO error midway through the read but still had to
			 * return the number of bytes that were actually read.
			 *
			 * We need to force an EIO, so try reading the rest of
			 * the block (if it was a partial block read) or the
			 * next full block.
			 */
			io_max_size = rvp->miniosz - (sz % rvp->miniosz);
			dbg_printf("SHORT %d READ @ %"PRIu64" %zu try for %zd\n",
					rvp->disk->d_fd, rv->io_start, sz,
					io_max_size);
		} else {
			/* We should never get back more bytes than we asked. */
			assert(sz == len);
		}

		progress_add(sz);
		if (read_error == 0)
			verified += sz;
		rv->io_start += sz;
		rv->io_length -= sz;
		background_sleep();
	}

	free(rv);
	ret = ptcounter_add(rvp->verified_bytes, verified);
	if (ret)
		rvp->runtime_error = ret;
}

/* Queue a read verify request. */
static int
read_verify_queue(
	struct read_verify_pool		*rvp,
	struct read_verify		*rv)
{
	struct read_verify		*tmp;
	bool				ret;

	dbg_printf("verify fd %d start %"PRIu64" len %"PRIu64"\n",
			rvp->disk->d_fd, rv->io_start, rv->io_length);

	/* Worker thread saw a runtime error, don't queue more. */
	if (rvp->runtime_error)
		return rvp->runtime_error;

	/* Otherwise clone the request and queue the copy. */
	tmp = malloc(sizeof(struct read_verify));
	if (!tmp) {
		rvp->runtime_error = errno;
		return errno;
	}

	memcpy(tmp, rv, sizeof(*tmp));

	ret = -workqueue_add(&rvp->wq, read_verify, 0, tmp);
	if (ret) {
		free(tmp);
		rvp->runtime_error = ret;
		return ret;
	}

	rv->io_length = 0;
	return 0;
}

/*
 * Issue an IO request.  We'll batch subsequent requests if they're
 * within 64k of each other
 */
int
read_verify_schedule_io(
	struct read_verify_pool		*rvp,
	uint64_t			start,
	uint64_t			length,
	void				*end_arg)
{
	struct read_verify		*rv;
	uint64_t			req_end;
	uint64_t			rv_end;
	int				ret;

	assert(rvp->readbuf);

	/* Round up and down to the start of a miniosz chunk. */
	start &= ~(rvp->miniosz - 1);
	length = roundup(length, rvp->miniosz);

	rv = ptvar_get(rvp->rvstate, &ret);
	if (ret)
		return -ret;
	req_end = start + length;
	rv_end = rv->io_start + rv->io_length;

	/*
	 * If we have a stashed IO, we haven't changed fds, the error
	 * reporting is the same, and the two extents are close,
	 * we can combine them.
	 */
	if (rv->io_length > 0 &&
	    end_arg == rv->io_end_arg &&
	    ((start >= rv->io_start && start <= rv_end + RVP_IO_BATCH_LOCALITY) ||
	     (rv->io_start >= start &&
	      rv->io_start <= req_end + RVP_IO_BATCH_LOCALITY))) {
		rv->io_start = min(rv->io_start, start);
		rv->io_length = max(req_end, rv_end) - rv->io_start;
	} else  {
		/* Otherwise, issue the stashed IO (if there is one) */
		if (rv->io_length > 0) {
			int	res;

			res = read_verify_queue(rvp, rv);
			if (res)
				return res;
		}

		/* Stash the new IO. */
		rv->io_start = start;
		rv->io_length = length;
		rv->io_end_arg = end_arg;
	}

	return 0;
}

/* Force any per-thread stashed IOs into the verifier. */
static int
force_one_io(
	struct ptvar		*ptv,
	void			*data,
	void			*foreach_arg)
{
	struct read_verify_pool	*rvp = foreach_arg;
	struct read_verify	*rv = data;

	if (rv->io_length == 0)
		return 0;

	return -read_verify_queue(rvp, rv);
}

/* Force any stashed IOs into the verifier. */
int
read_verify_force_io(
	struct read_verify_pool		*rvp)
{
	assert(rvp->readbuf);

	return -ptvar_foreach(rvp->rvstate, force_one_io, rvp);
}

/* How many bytes has this process verified? */
int
read_verify_bytes(
	struct read_verify_pool		*rvp,
	uint64_t			*bytes_checked)
{
	return ptcounter_value(rvp->verified_bytes, bytes_checked);
}
