// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Oracle, Inc.
 * All Rights Reserved.
 */
#include "xfs.h"
#include "fsgeom.h"
#include "scrub.h"

/* These must correspond to XFS_SCRUB_TYPE_ */
const struct xfrog_scrub_descr xfrog_scrubbers[XFS_SCRUB_TYPE_NR] = {
	[XFS_SCRUB_TYPE_PROBE] = {
		.name	= "probe",
		.descr	= "metadata",
		.type	= XFROG_SCRUB_TYPE_NONE,
	},
	[XFS_SCRUB_TYPE_SB] = {
		.name	= "sb",
		.descr	= "superblock",
		.type	= XFROG_SCRUB_TYPE_AGHEADER,
	},
	[XFS_SCRUB_TYPE_AGF] = {
		.name	= "agf",
		.descr	= "free space header",
		.type	= XFROG_SCRUB_TYPE_AGHEADER,
	},
	[XFS_SCRUB_TYPE_AGFL] = {
		.name	= "agfl",
		.descr	= "free list",
		.type	= XFROG_SCRUB_TYPE_AGHEADER,
	},
	[XFS_SCRUB_TYPE_AGI] = {
		.name	= "agi",
		.descr	= "inode header",
		.type	= XFROG_SCRUB_TYPE_AGHEADER,
	},
	[XFS_SCRUB_TYPE_BNOBT] = {
		.name	= "bnobt",
		.descr	= "freesp by block btree",
		.type	= XFROG_SCRUB_TYPE_PERAG,
	},
	[XFS_SCRUB_TYPE_CNTBT] = {
		.name	= "cntbt",
		.descr	= "freesp by length btree",
		.type	= XFROG_SCRUB_TYPE_PERAG,
	},
	[XFS_SCRUB_TYPE_INOBT] = {
		.name	= "inobt",
		.descr	= "inode btree",
		.type	= XFROG_SCRUB_TYPE_PERAG,
	},
	[XFS_SCRUB_TYPE_FINOBT] = {
		.name	= "finobt",
		.descr	= "free inode btree",
		.type	= XFROG_SCRUB_TYPE_PERAG,
	},
	[XFS_SCRUB_TYPE_RMAPBT] = {
		.name	= "rmapbt",
		.descr	= "reverse mapping btree",
		.type	= XFROG_SCRUB_TYPE_PERAG,
	},
	[XFS_SCRUB_TYPE_REFCNTBT] = {
		.name	= "refcountbt",
		.descr	= "reference count btree",
		.type	= XFROG_SCRUB_TYPE_PERAG,
	},
	[XFS_SCRUB_TYPE_INODE] = {
		.name	= "inode",
		.descr	= "inode record",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_BMBTD] = {
		.name	= "bmapbtd",
		.descr	= "data block map",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_BMBTA] = {
		.name	= "bmapbta",
		.descr	= "attr block map",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_BMBTC] = {
		.name	= "bmapbtc",
		.descr	= "CoW block map",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_DIR] = {
		.name	= "directory",
		.descr	= "directory entries",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_XATTR] = {
		.name	= "xattr",
		.descr	= "extended attributes",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_SYMLINK] = {
		.name	= "symlink",
		.descr	= "symbolic link",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_PARENT] = {
		.name	= "parent",
		.descr	= "parent pointer",
		.type	= XFROG_SCRUB_TYPE_INODE,
	},
	[XFS_SCRUB_TYPE_RTBITMAP] = {
		.name	= "rtbitmap",
		.descr	= "realtime bitmap",
		.type	= XFROG_SCRUB_TYPE_FS,
	},
	[XFS_SCRUB_TYPE_RTSUM] = {
		.name	= "rtsummary",
		.descr	= "realtime summary",
		.type	= XFROG_SCRUB_TYPE_FS,
	},
	[XFS_SCRUB_TYPE_UQUOTA] = {
		.name	= "usrquota",
		.descr	= "user quotas",
		.type	= XFROG_SCRUB_TYPE_FS,
	},
	[XFS_SCRUB_TYPE_GQUOTA] = {
		.name	= "grpquota",
		.descr	= "group quotas",
		.type	= XFROG_SCRUB_TYPE_FS,
	},
	[XFS_SCRUB_TYPE_PQUOTA] = {
		.name	= "prjquota",
		.descr	= "project quotas",
		.type	= XFROG_SCRUB_TYPE_FS,
	},
	[XFS_SCRUB_TYPE_FSCOUNTERS] = {
		.name	= "fscounters",
		.descr	= "filesystem summary counters",
		.type	= XFROG_SCRUB_TYPE_FS,
		.flags	= XFROG_SCRUB_DESCR_SUMMARY,
	},
};

/* Invoke the scrub ioctl.  Returns zero or negative error code. */
int
xfrog_scrub_metadata(
	struct xfs_fd			*xfd,
	struct xfs_scrub_metadata	*meta)
{
	int				ret;

	ret = ioctl(xfd->fd, XFS_IOC_SCRUB_METADATA, meta);
	if (ret)
		return -errno;

	return 0;
}
