// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2005 Silicon Graphics, Inc.  All Rights Reserved.
 */
#ifndef __LIBFROG_FSGEOM_H__
#define __LIBFROG_FSGEOM_H__

void xfs_report_geom(struct xfs_fsop_geom *geo, const char *mntpoint,
		const char *logname, const char *rtname);
int xfrog_geometry(int fd, struct xfs_fsop_geom *fsgeo);
int xfrog_ag_geometry(int fd, unsigned int agno, struct xfs_ag_geometry *ageo);

/*
 * Structure for recording whatever observations we want about the level of
 * xfs runtime support for this fd.  Right now we only store the fd and fs
 * geometry.
 */
struct xfs_fd {
	/* ioctl file descriptor */
	int			fd;

	/* filesystem geometry */
	struct xfs_fsop_geom	fsgeom;

	/* log2 of sb_agblocks (rounded up) */
	unsigned int		agblklog;

	/* log2 of sb_blocksize */
	unsigned int		blocklog;

	/* log2 of sb_inodesize */
	unsigned int		inodelog;

	/* log2 of sb_inopblock */
	unsigned int		inopblog;

	/* bits for agino in inum */
	unsigned int		aginolog;

	/* log2 of sb_blocksize / sb_sectsize */
	unsigned int		blkbb_log;

	/* XFROG_FLAG_* state flags */
	unsigned int		flags;
};

/* Only use v1 bulkstat/inumbers ioctls. */
#define XFROG_FLAG_BULKSTAT_FORCE_V1	(1 << 0)

/* Only use v5 bulkstat/inumbers ioctls. */
#define XFROG_FLAG_BULKSTAT_FORCE_V5	(1 << 1)

/* Static initializers */
#define XFS_FD_INIT(_fd)	{ .fd = (_fd), }
#define XFS_FD_INIT_EMPTY	XFS_FD_INIT(-1)

int xfd_prepare_geometry(struct xfs_fd *xfd);
int xfd_open(struct xfs_fd *xfd, const char *pathname, int flags);
int xfd_close(struct xfs_fd *xfd);

/* Convert AG number and AG inode number into fs inode number. */
static inline uint64_t
cvt_agino_to_ino(
	const struct xfs_fd	*xfd,
	uint32_t		agno,
	uint32_t		agino)
{
	return ((uint64_t)agno << xfd->aginolog) + agino;
}

/* Convert fs inode number into AG number. */
static inline uint32_t
cvt_ino_to_agno(
	const struct xfs_fd	*xfd,
	uint64_t		ino)
{
	return ino >> xfd->aginolog;
}

/* Convert fs inode number into AG inode number. */
static inline uint32_t
cvt_ino_to_agino(
	const struct xfs_fd	*xfd,
	uint64_t		ino)
{
	return ino & ((1ULL << xfd->aginolog) - 1);
}

/*
 * Convert a linear fs block offset number into bytes.  This is the runtime
 * equivalent of XFS_FSB_TO_B, which means that it is /not/ for segmented fsbno
 * format (= agno | agbno) that we use internally for the data device.
 */
static inline uint64_t
cvt_off_fsb_to_b(
	const struct xfs_fd	*xfd,
	uint64_t		fsb)
{
	return fsb << xfd->blocklog;
}

/*
 * Convert bytes into a (rounded down) linear fs block offset number.  This is
 * the runtime equivalent of XFS_B_TO_FSBT.  It does not produce segmented
 * fsbno numbers (= agno | agbno).
 */
static inline uint64_t
cvt_b_to_off_fsbt(
	const struct xfs_fd	*xfd,
	uint64_t		bytes)
{
	return bytes >> xfd->blocklog;
}

/* Convert sector number to bytes. */
static inline uint64_t
cvt_bbtob(
	uint64_t		daddr)
{
	return daddr << BBSHIFT;
}

/* Convert bytes to sector number, rounding down. */
static inline uint64_t
cvt_btobbt(
	uint64_t		bytes)
{
	return bytes >> BBSHIFT;
}

/* Convert fs block number to sector number. */
static inline uint64_t
cvt_off_fsb_to_bb(
	struct xfs_fd		*xfd,
	uint64_t		fsbno)
{
	return fsbno << xfd->blkbb_log;
}

/* Convert sector number to fs block number, rounded down. */
static inline uint64_t
cvt_bb_to_off_fsbt(
	struct xfs_fd		*xfd,
	uint64_t		daddr)
{
	return daddr >> xfd->blkbb_log;
}

/* Convert AG number and AG block to fs block number */
static inline uint64_t
cvt_agb_to_daddr(
	struct xfs_fd		*xfd,
	uint32_t		agno,
	uint32_t		agbno)
{
	return cvt_off_fsb_to_bb(xfd,
			(uint64_t)agno * xfd->fsgeom.agblocks + agbno);
}

/* Convert sector number to AG number. */
static inline uint32_t
cvt_daddr_to_agno(
	struct xfs_fd		*xfd,
	uint64_t		daddr)
{
	return cvt_bb_to_off_fsbt(xfd, daddr) / xfd->fsgeom.agblocks;
}

/* Convert sector number to AG block number. */
static inline uint32_t
cvt_daddr_to_agbno(
	struct xfs_fd		*xfd,
	uint64_t		daddr)
{
	return cvt_bb_to_off_fsbt(xfd, daddr) % xfd->fsgeom.agblocks;
}

/* Convert AG number and AG block to a byte location on disk. */
static inline uint64_t
cvt_agbno_to_b(
	struct xfs_fd		*xfd,
	xfs_agnumber_t		agno,
	xfs_agblock_t		agbno)
{
	return cvt_bbtob(cvt_agb_to_daddr(xfd, agno, agbno));
}

/* Convert byte location on disk to AG block. */
static inline xfs_agblock_t
cvt_b_to_agbno(
	struct xfs_fd		*xfd,
	uint64_t		byteno)
{
	return cvt_daddr_to_agbno(xfd, cvt_btobbt(byteno));
}

#endif /* __LIBFROG_FSGEOM_H__ */
