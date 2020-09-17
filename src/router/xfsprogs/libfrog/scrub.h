/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Oracle, Inc.
 * All Rights Reserved.
 */
#ifndef __LIBFROG_SCRUB_H__
#define __LIBFROG_SCRUB_H__

/* Type info and names for the scrub types. */
enum xfrog_scrub_type {
	XFROG_SCRUB_TYPE_NONE,		/* not metadata */
	XFROG_SCRUB_TYPE_AGHEADER,	/* per-AG header */
	XFROG_SCRUB_TYPE_PERAG,		/* per-AG metadata */
	XFROG_SCRUB_TYPE_FS,		/* per-FS metadata */
	XFROG_SCRUB_TYPE_INODE,		/* per-inode metadata */
};

/* Catalog of scrub types and names, indexed by XFS_SCRUB_TYPE_* */
struct xfrog_scrub_descr {
	const char		*name;
	const char		*descr;
	enum xfrog_scrub_type	type;
	unsigned int		flags;
};

/*
 * The type of metadata checked by this scrubber is a summary of other types
 * of metadata.  This scrubber should be run after all the others.
 */
#define XFROG_SCRUB_DESCR_SUMMARY	(1 << 0)

extern const struct xfrog_scrub_descr xfrog_scrubbers[XFS_SCRUB_TYPE_NR];

int xfrog_scrub_metadata(struct xfs_fd *xfd, struct xfs_scrub_metadata *meta);

#endif	/* __LIBFROG_SCRUB_H__ */
