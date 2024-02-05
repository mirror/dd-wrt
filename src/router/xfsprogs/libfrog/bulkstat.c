// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include <string.h>
#include <strings.h>
#include "xfs.h"
#include "fsgeom.h"
#include "bulkstat.h"

/*
 * Wrapper functions for BULKSTAT and INUMBERS
 * ===========================================
 *
 * The functions in this file are thin wrappers around the most recent version
 * of the BULKSTAT and INUMBERS ioctls.  BULKSTAT is used to query XFS-specific
 * stat information about a group of inodes.  INUMBERS is used to query
 * allocation information about batches of XFS inodes.
 *
 * At the moment, the public xfrog_* functions provide all functionality of the
 * V5 interface.  If the V5 interface is not available on the running kernel,
 * the functions will emulate them as best they can with previous versions of
 * the interface (currently V1).  If emulation is not possible, EINVAL will be
 * returned.
 *
 * The XFROG_FLAG_BULKSTAT_FORCE_V[15] flags can be used to force use of a
 * particular version of the kernel interface for testing.
 */

/*
 * Grab the fs geometry information that is needed to needed to emulate v5 with
 * v1 interfaces.
 */
static inline int
xfrog_bulkstat_prep_v1_emulation(
	struct xfs_fd		*xfd)
{
	if (xfd->fsgeom.blocksize > 0)
		return 0;

	return xfd_prepare_geometry(xfd);
}

/* Bulkstat a single inode using v5 ioctl. */
static int
xfrog_bulkstat_single5(
	struct xfs_fd			*xfd,
	uint64_t			ino,
	unsigned int			flags,
	struct xfs_bulkstat		*bulkstat)
{
	struct xfs_bulkstat_req		*req;
	int				ret;

	if (flags & ~(XFS_BULK_IREQ_SPECIAL | XFS_BULK_IREQ_NREXT64))
		return -EINVAL;

	if (xfd->fsgeom.flags & XFS_FSOP_GEOM_FLAGS_NREXT64)
		flags |= XFS_BULK_IREQ_NREXT64;

	ret = xfrog_bulkstat_alloc_req(1, ino, &req);
	if (ret)
		return ret;

	req->hdr.flags = flags;
	ret = ioctl(xfd->fd, XFS_IOC_BULKSTAT, req);
	if (ret) {
		ret = -errno;
		goto free;
	}

	if (req->hdr.ocount == 0) {
		ret = -ENOENT;
		goto free;
	}

	memcpy(bulkstat, req->bulkstat, sizeof(struct xfs_bulkstat));

	if (!(xfd->fsgeom.flags & XFS_FSOP_GEOM_FLAGS_NREXT64)) {
		bulkstat->bs_extents64 = bulkstat->bs_extents;
		bulkstat->bs_extents = 0;
	}

free:
	free(req);
	return ret;
}

/* Bulkstat a single inode using v1 ioctl. */
static int
xfrog_bulkstat_single1(
	struct xfs_fd			*xfd,
	uint64_t			ino,
	unsigned int			flags,
	struct xfs_bulkstat		*bulkstat)
{
	struct xfs_bstat		bstat;
	struct xfs_fsop_bulkreq		bulkreq = { 0 };
	int				error;

	if (flags)
		return -EINVAL;

	error = xfrog_bulkstat_prep_v1_emulation(xfd);
	if (error)
		return error;

	bulkreq.lastip = (__u64 *)&ino;
	bulkreq.icount = 1;
	bulkreq.ubuffer = &bstat;
	error = ioctl(xfd->fd, XFS_IOC_FSBULKSTAT_SINGLE, &bulkreq);
	if (error)
		return -errno;

	xfrog_bulkstat_v1_to_v5(xfd, bulkstat, &bstat);
	return 0;
}

/* Bulkstat a single inode.  Returns zero or a negative error code. */
int
xfrog_bulkstat_single(
	struct xfs_fd			*xfd,
	uint64_t			ino,
	unsigned int			flags,
	struct xfs_bulkstat		*bulkstat)
{
	int				error;

	if (xfd->flags & XFROG_FLAG_BULKSTAT_FORCE_V1)
		goto try_v1;

	error = xfrog_bulkstat_single5(xfd, ino, flags, bulkstat);
	if (error == 0 || (xfd->flags & XFROG_FLAG_BULKSTAT_FORCE_V5))
		return error;

	/* If the v5 ioctl wasn't found, we punt to v1. */
	switch (error) {
	case -EOPNOTSUPP:
	case -ENOTTY:
		assert(!(xfd->fsgeom.flags & XFS_FSOP_GEOM_FLAGS_NREXT64));
		xfd->flags |= XFROG_FLAG_BULKSTAT_FORCE_V1;
		break;
	}

try_v1:
	return xfrog_bulkstat_single1(xfd, ino, flags, bulkstat);
}

/*
 * Set up the necessary control structures to emulate a V5 bulk request ioctl
 * by calling a V1 bulk request ioctl.  This enables callers to run on older
 * kernels.
 *
 * Returns 0 if the emulation should proceed; ECANCELED if there are no
 * records; or a negative error code.
 */
static int
xfrog_bulk_req_v1_setup(
	struct xfs_fd		*xfd,
	struct xfs_bulk_ireq	*hdr,
	struct xfs_fsop_bulkreq	*bulkreq,
	size_t			rec_size)
{
	void			*buf;

	if (hdr->flags & XFS_BULK_IREQ_AGNO) {
		uint32_t	agno = cvt_ino_to_agno(xfd, hdr->ino);

		if (hdr->ino == 0)
			hdr->ino = cvt_agino_to_ino(xfd, hdr->agno, 0);
		else if (agno < hdr->agno)
			return -EINVAL;
		else if (agno > hdr->agno)
			goto no_results;
	}

	if (cvt_ino_to_agno(xfd, hdr->ino) > xfd->fsgeom.agcount)
		goto no_results;

	buf = malloc(hdr->icount * rec_size);
	if (!buf)
		return -errno;

	if (hdr->ino)
		hdr->ino--;
	bulkreq->lastip = (__u64 *)&hdr->ino,
	bulkreq->icount = hdr->icount,
	bulkreq->ocount = (__s32 *)&hdr->ocount,
	bulkreq->ubuffer = buf;
	return 0;

no_results:
	hdr->ocount = 0;
	return -ECANCELED;
}

/*
 * Clean up after using a V1 bulk request to emulate a V5 bulk request call.
 *
 * If the ioctl was successful, we need to convert the returned V1-format bulk
 * request data into the V5-format bulk request data and copy it into the
 * caller's buffer.  We also need to free all resources allocated during the
 * setup setup.
 */
static int
xfrog_bulk_req_v1_cleanup(
	struct xfs_fd		*xfd,
	struct xfs_bulk_ireq	*hdr,
	struct xfs_fsop_bulkreq	*bulkreq,
	size_t			v1_rec_size,
	uint64_t		(*v1_ino)(void *v1_rec),
	void			*v5_records,
	size_t			v5_rec_size,
	void			(*cvt)(struct xfs_fd *xfd, void *v5, void *v1),
	unsigned int		startino_adj,
	int			error)
{
	void			*v1_rec = bulkreq->ubuffer;
	void			*v5_rec = v5_records;
	unsigned int		i;

	if (error == -ECANCELED) {
		error = 0;
		goto free;
	}
	if (error)
		goto free;

	/*
	 * Convert each record from v1 to v5 format, keeping the startino
	 * value up to date and (if desired) stopping at the end of the
	 * AG.
	 */
	for (i = 0;
	     i < hdr->ocount;
	     i++, v1_rec += v1_rec_size, v5_rec += v5_rec_size) {
		uint64_t	ino = v1_ino(v1_rec);

		/* Stop if we hit a different AG. */
		if ((hdr->flags & XFS_BULK_IREQ_AGNO) &&
		    cvt_ino_to_agno(xfd, ino) != hdr->agno) {
			hdr->ocount = i;
			break;
		}
		cvt(xfd, v5_rec, v1_rec);
		hdr->ino = ino + startino_adj;
	}

free:
	free(bulkreq->ubuffer);
	return error;
}

static uint64_t xfrog_bstat_ino(void *v1_rec)
{
	return ((struct xfs_bstat *)v1_rec)->bs_ino;
}

static void xfrog_bstat_cvt(struct xfs_fd *xfd, void *v5, void *v1)
{
	xfrog_bulkstat_v1_to_v5(xfd, v5, v1);
}

/* Bulkstat a bunch of inodes using the v5 interface. */
static int
xfrog_bulkstat5(
	struct xfs_fd		*xfd,
	struct xfs_bulkstat_req	*req)
{
	int			ret;
	int			i;

	if (xfd->fsgeom.flags & XFS_FSOP_GEOM_FLAGS_NREXT64)
		req->hdr.flags |= XFS_BULK_IREQ_NREXT64;

	ret = ioctl(xfd->fd, XFS_IOC_BULKSTAT, req);
	if (ret)
		return -errno;

	if (!(xfd->fsgeom.flags & XFS_FSOP_GEOM_FLAGS_NREXT64)) {
		for (i = 0; i < req->hdr.ocount; i++) {
			req->bulkstat[i].bs_extents64 =
				req->bulkstat[i].bs_extents;
			req->bulkstat[i].bs_extents = 0;
		}
	}

	return 0;
}

/* Bulkstat a bunch of inodes using the v1 interface. */
static int
xfrog_bulkstat1(
	struct xfs_fd		*xfd,
	struct xfs_bulkstat_req	*req)
{
	struct xfs_fsop_bulkreq	bulkreq = { 0 };
	int			error;

	error = xfrog_bulkstat_prep_v1_emulation(xfd);
	if (error)
		return error;

	error = xfrog_bulk_req_v1_setup(xfd, &req->hdr, &bulkreq,
			sizeof(struct xfs_bstat));
	if (error == -ECANCELED)
		goto out_teardown;
	if (error)
		return error;

	error = ioctl(xfd->fd, XFS_IOC_FSBULKSTAT, &bulkreq);
	if (error)
		error = -errno;

out_teardown:
	return xfrog_bulk_req_v1_cleanup(xfd, &req->hdr, &bulkreq,
			sizeof(struct xfs_bstat), xfrog_bstat_ino,
			&req->bulkstat, sizeof(struct xfs_bulkstat),
			xfrog_bstat_cvt, 1, error);
}

/* Bulkstat a bunch of inodes.  Returns zero or a positive error code. */
int
xfrog_bulkstat(
	struct xfs_fd		*xfd,
	struct xfs_bulkstat_req	*req)
{
	int			error;

	if (xfd->flags & XFROG_FLAG_BULKSTAT_FORCE_V1)
		goto try_v1;

	error = xfrog_bulkstat5(xfd, req);
	if (error == 0 || (xfd->flags & XFROG_FLAG_BULKSTAT_FORCE_V5))
		return error;

	/* If the v5 ioctl wasn't found, we punt to v1. */
	switch (error) {
	case -EOPNOTSUPP:
	case -ENOTTY:
		assert(!(xfd->fsgeom.flags & XFS_FSOP_GEOM_FLAGS_NREXT64));
		xfd->flags |= XFROG_FLAG_BULKSTAT_FORCE_V1;
		break;
	}

try_v1:
	return xfrog_bulkstat1(xfd, req);
}

static bool
time_too_big(
	uint64_t	time)
{
	time_t		TIME_MAX;

	memset(&TIME_MAX, 0xFF, sizeof(TIME_MAX));
	return time > TIME_MAX;
}

/* Convert bulkstat data from v5 format to v1 format. */
int
xfrog_bulkstat_v5_to_v1(
	struct xfs_fd			*xfd,
	struct xfs_bstat		*bs1,
	const struct xfs_bulkstat	*bs5)
{
	if (bs5->bs_aextents > UINT16_MAX ||
	    bs5->bs_extents64 > INT32_MAX ||
	    cvt_off_fsb_to_b(xfd, bs5->bs_extsize_blks) > UINT32_MAX ||
	    cvt_off_fsb_to_b(xfd, bs5->bs_cowextsize_blks) > UINT32_MAX ||
	    time_too_big(bs5->bs_atime) ||
	    time_too_big(bs5->bs_ctime) ||
	    time_too_big(bs5->bs_mtime))
		return -ERANGE;

	bs1->bs_ino = bs5->bs_ino;
	bs1->bs_mode = bs5->bs_mode;
	bs1->bs_nlink = bs5->bs_nlink;
	bs1->bs_uid = bs5->bs_uid;
	bs1->bs_gid = bs5->bs_gid;
	bs1->bs_rdev = bs5->bs_rdev;
	bs1->bs_blksize = bs5->bs_blksize;
	bs1->bs_size = bs5->bs_size;
	bs1->bs_atime.tv_sec = bs5->bs_atime;
	bs1->bs_mtime.tv_sec = bs5->bs_mtime;
	bs1->bs_ctime.tv_sec = bs5->bs_ctime;
	bs1->bs_atime.tv_nsec = bs5->bs_atime_nsec;
	bs1->bs_mtime.tv_nsec = bs5->bs_mtime_nsec;
	bs1->bs_ctime.tv_nsec = bs5->bs_ctime_nsec;
	bs1->bs_blocks = bs5->bs_blocks;
	bs1->bs_xflags = bs5->bs_xflags;
	bs1->bs_extsize = cvt_off_fsb_to_b(xfd, bs5->bs_extsize_blks);
	bs1->bs_extents = bs5->bs_extents64;
	bs1->bs_gen = bs5->bs_gen;
	bs1->bs_projid_lo = bs5->bs_projectid & 0xFFFF;
	bs1->bs_forkoff = bs5->bs_forkoff;
	bs1->bs_projid_hi = bs5->bs_projectid >> 16;
	bs1->bs_sick = bs5->bs_sick;
	bs1->bs_checked = bs5->bs_checked;
	bs1->bs_cowextsize = cvt_off_fsb_to_b(xfd, bs5->bs_cowextsize_blks);
	bs1->bs_dmevmask = 0;
	bs1->bs_dmstate = 0;
	bs1->bs_aextents = bs5->bs_aextents;
	return 0;
}

/* Convert bulkstat data from v1 format to v5 format. */
void
xfrog_bulkstat_v1_to_v5(
	struct xfs_fd			*xfd,
	struct xfs_bulkstat		*bs5,
	const struct xfs_bstat		*bs1)
{
	memset(bs5, 0, sizeof(*bs5));
	bs5->bs_version = XFS_BULKSTAT_VERSION_V1;

	bs5->bs_ino = bs1->bs_ino;
	bs5->bs_mode = bs1->bs_mode;
	bs5->bs_nlink = bs1->bs_nlink;
	bs5->bs_uid = bs1->bs_uid;
	bs5->bs_gid = bs1->bs_gid;
	bs5->bs_rdev = bs1->bs_rdev;
	bs5->bs_blksize = bs1->bs_blksize;
	bs5->bs_size = bs1->bs_size;
	bs5->bs_atime = bs1->bs_atime.tv_sec;
	bs5->bs_mtime = bs1->bs_mtime.tv_sec;
	bs5->bs_ctime = bs1->bs_ctime.tv_sec;
	bs5->bs_atime_nsec = bs1->bs_atime.tv_nsec;
	bs5->bs_mtime_nsec = bs1->bs_mtime.tv_nsec;
	bs5->bs_ctime_nsec = bs1->bs_ctime.tv_nsec;
	bs5->bs_blocks = bs1->bs_blocks;
	bs5->bs_xflags = bs1->bs_xflags;
	bs5->bs_extsize_blks = cvt_b_to_off_fsbt(xfd, bs1->bs_extsize);
	bs5->bs_gen = bs1->bs_gen;
	bs5->bs_projectid = bstat_get_projid(bs1);
	bs5->bs_forkoff = bs1->bs_forkoff;
	bs5->bs_sick = bs1->bs_sick;
	bs5->bs_checked = bs1->bs_checked;
	bs5->bs_cowextsize_blks = cvt_b_to_off_fsbt(xfd, bs1->bs_cowextsize);
	bs5->bs_aextents = bs1->bs_aextents;
	bs5->bs_extents64 = bs1->bs_extents;
}

/* Allocate a bulkstat request.  Returns zero or a negative error code. */
int
xfrog_bulkstat_alloc_req(
	uint32_t		nr,
	uint64_t		startino,
	struct xfs_bulkstat_req **preq)
{
	struct xfs_bulkstat_req	*breq;

	breq = calloc(1, XFS_BULKSTAT_REQ_SIZE(nr));
	if (!breq)
		return -ENOMEM;

	breq->hdr.icount = nr;
	breq->hdr.ino = startino;

	*preq = breq;
	return 0;
}

/* Set a bulkstat cursor to iterate only a particular AG. */
void
xfrog_bulkstat_set_ag(
	struct xfs_bulkstat_req	*req,
	uint32_t		agno)
{
	req->hdr.agno = agno;
	req->hdr.flags |= XFS_BULK_IREQ_AGNO;
}

/* Convert a inumbers data from v5 format to v1 format. */
void
xfrog_inumbers_v5_to_v1(
	struct xfs_inogrp		*ig1,
	const struct xfs_inumbers	*ig5)
{
	ig1->xi_startino = ig5->xi_startino;
	ig1->xi_alloccount = ig5->xi_alloccount;
	ig1->xi_allocmask = ig5->xi_allocmask;
}

/* Convert a inumbers data from v1 format to v5 format. */
void
xfrog_inumbers_v1_to_v5(
	struct xfs_inumbers		*ig5,
	const struct xfs_inogrp		*ig1)
{
	memset(ig5, 0, sizeof(*ig5));
	ig5->xi_version = XFS_INUMBERS_VERSION_V1;

	ig5->xi_startino = ig1->xi_startino;
	ig5->xi_alloccount = ig1->xi_alloccount;
	ig5->xi_allocmask = ig1->xi_allocmask;
}

static uint64_t xfrog_inum_ino(void *v1_rec)
{
	return ((struct xfs_inogrp *)v1_rec)->xi_startino;
}

static void xfrog_inum_cvt(struct xfs_fd *xfd, void *v5, void *v1)
{
	xfrog_inumbers_v1_to_v5(v5, v1);
}

/* Query inode allocation bitmask information using v5 ioctl. */
static int
xfrog_inumbers5(
	struct xfs_fd		*xfd,
	struct xfs_inumbers_req	*req)
{
	int			ret;

	ret = ioctl(xfd->fd, XFS_IOC_INUMBERS, req);
	if (ret)
		return -errno;
	return 0;
}

/* Query inode allocation bitmask information using v1 ioctl. */
static int
xfrog_inumbers1(
	struct xfs_fd		*xfd,
	struct xfs_inumbers_req	*req)
{
	struct xfs_fsop_bulkreq	bulkreq = { 0 };
	int			error;

	error = xfrog_bulkstat_prep_v1_emulation(xfd);
	if (error)
		return error;

	error = xfrog_bulk_req_v1_setup(xfd, &req->hdr, &bulkreq,
			sizeof(struct xfs_inogrp));
	if (error == -ECANCELED)
		goto out_teardown;
	if (error)
		return error;

	error = ioctl(xfd->fd, XFS_IOC_FSINUMBERS, &bulkreq);
	if (error)
		error = -errno;

out_teardown:
	return xfrog_bulk_req_v1_cleanup(xfd, &req->hdr, &bulkreq,
			sizeof(struct xfs_inogrp), xfrog_inum_ino,
			&req->inumbers, sizeof(struct xfs_inumbers),
			xfrog_inum_cvt, 64, error);
}

/*
 * Query inode allocation bitmask information.  Returns zero or a negative
 * error code.
 */
int
xfrog_inumbers(
	struct xfs_fd		*xfd,
	struct xfs_inumbers_req	*req)
{
	int			error;

	if (xfd->flags & XFROG_FLAG_BULKSTAT_FORCE_V1)
		goto try_v1;

	error = xfrog_inumbers5(xfd, req);
	if (error == 0 || (xfd->flags & XFROG_FLAG_BULKSTAT_FORCE_V5))
		return error;

	/* If the v5 ioctl wasn't found, we punt to v1. */
	switch (error) {
	case -EOPNOTSUPP:
	case -ENOTTY:
		xfd->flags |= XFROG_FLAG_BULKSTAT_FORCE_V1;
		break;
	}

try_v1:
	return xfrog_inumbers1(xfd, req);
}

/* Allocate a inumbers request.  Returns zero or a negative error code. */
int
xfrog_inumbers_alloc_req(
	uint32_t		nr,
	uint64_t		startino,
	struct xfs_inumbers_req **preq)
{
	struct xfs_inumbers_req	*ireq;

	ireq = calloc(1, XFS_INUMBERS_REQ_SIZE(nr));
	if (!ireq)
		return -errno;

	ireq->hdr.icount = nr;
	ireq->hdr.ino = startino;

	*preq = ireq;
	return 0;
}

/* Set an inumbers cursor to iterate only a particular AG. */
void
xfrog_inumbers_set_ag(
	struct xfs_inumbers_req	*req,
	uint32_t		agno)
{
	req->hdr.agno = agno;
	req->hdr.flags |= XFS_BULK_IREQ_AGNO;
}
