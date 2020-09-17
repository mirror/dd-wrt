// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include "platform_defs.h"
#include "xfs_arch.h"
#include "xfs_format.h"
#include "libfrog/paths.h"
#include "libfrog/workqueue.h"
#include "xfs_scrub.h"
#include "common.h"
#include "fscounters.h"
#include "libfrog/bulkstat.h"

/*
 * Filesystem counter collection routines.  We can count the number of
 * inodes in the filesystem, and we can estimate the block counters.
 */

/* Count the number of inodes in the filesystem. */

/* INUMBERS wrapper routines. */
struct count_inodes {
	int			error;
	uint64_t		counters[0];
};

/*
 * Count the number of inodes.  Use INUMBERS to figure out how many inodes
 * exist in the filesystem, assuming we've already scrubbed that.
 */
static void
count_ag_inodes(
	struct workqueue	*wq,
	xfs_agnumber_t		agno,
	void			*arg)
{
	struct count_inodes	*ci = arg;
	struct scrub_ctx	*ctx = (struct scrub_ctx *)wq->wq_ctx;
	struct xfs_inumbers_req	*ireq;
	uint64_t		nr = 0;
	unsigned int		i;
	int			error;

	error = -xfrog_inumbers_alloc_req(64, 0, &ireq);
	if (error) {
		ci->error = error;
		return;
	}
	xfrog_inumbers_set_ag(ireq, agno);

	while (!ci->error && (error = -xfrog_inumbers(&ctx->mnt, ireq)) == 0) {
		if (ireq->hdr.ocount == 0)
			break;
		for (i = 0; i < ireq->hdr.ocount; i++)
			nr += ireq->inumbers[i].xi_alloccount;
	}
	if (error)
		ci->error = error;

	free(ireq);

	ci->counters[agno] = nr;
}

/*
 * Count all the inodes in a filesystem.  Returns 0 or a positive error number.
 */
int
scrub_count_all_inodes(
	struct scrub_ctx	*ctx,
	uint64_t		*count)
{
	struct count_inodes	*ci;
	xfs_agnumber_t		agno;
	struct workqueue	wq;
	int			ret, ret2;

	ci = calloc(1, sizeof(struct count_inodes) +
			(ctx->mnt.fsgeom.agcount * sizeof(uint64_t)));
	if (!ci)
		return errno;

	ret = -workqueue_create(&wq, (struct xfs_mount *)ctx,
			scrub_nproc_workqueue(ctx));
	if (ret)
		goto out_free;

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount && !ci->error; agno++) {
		ret = -workqueue_add(&wq, count_ag_inodes, agno, ci);
		if (ret)
			break;
	}

	ret2 = -workqueue_terminate(&wq);
	if (!ret && ret2)
		ret = ret2;
	workqueue_destroy(&wq);

	if (ci->error) {
		ret = ci->error;
		goto out_free;
	}

	for (agno = 0; agno < ctx->mnt.fsgeom.agcount; agno++)
		*count += ci->counters[agno];

out_free:
	free(ci);
	return ret;
}

/*
 * Estimate the number of blocks and inodes in the filesystem.  Returns 0
 * or a positive error number.
 */
int
scrub_scan_estimate_blocks(
	struct scrub_ctx		*ctx,
	unsigned long long		*d_blocks,
	unsigned long long		*d_bfree,
	unsigned long long		*r_blocks,
	unsigned long long		*r_bfree,
	unsigned long long		*f_files,
	unsigned long long		*f_free)
{
	struct xfs_fsop_counts		fc;
	struct xfs_fsop_resblks		rb;
	struct statvfs			sfs;
	int				error;

	/* Grab the fstatvfs counters, since it has to report accurately. */
	error = fstatvfs(ctx->mnt.fd, &sfs);
	if (error)
		return errno;

	/* Fetch the filesystem counters. */
	error = ioctl(ctx->mnt.fd, XFS_IOC_FSCOUNTS, &fc);
	if (error)
		return errno;

	/*
	 * XFS reserves some blocks to prevent hard ENOSPC, so add those
	 * blocks back to the free data counts.
	 */
	error = ioctl(ctx->mnt.fd, XFS_IOC_GET_RESBLKS, &rb);
	if (error)
		return errno;

	sfs.f_bfree += rb.resblks_avail;

	*d_blocks = sfs.f_blocks;
	if (ctx->mnt.fsgeom.logstart > 0)
		*d_blocks += ctx->mnt.fsgeom.logblocks;
	*d_bfree = sfs.f_bfree;
	*r_blocks = ctx->mnt.fsgeom.rtblocks;
	*r_bfree = fc.freertx;
	*f_files = sfs.f_files;
	*f_free = sfs.f_ffree;

	return 0;
}
