// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_FILEMAP_H_
#define XFS_SCRUB_FILEMAP_H_

/* inode fork block mapping */
struct file_bmap {
	uint64_t	bm_offset;	/* file offset of segment in bytes */
	uint64_t	bm_physical;	/* physical starting byte  */
	uint64_t	bm_length;	/* length of segment, bytes */
	uint32_t	bm_flags;	/* output flags */
};

/*
 * Visit each inode fork mapping.  Return 0 to continue iteration or a positive
 * error code to stop iterating and return to the caller.
 */
typedef int (*scrub_bmap_iter_fn)(struct scrub_ctx *ctx, int fd, int whichfork,
		struct fsxattr *fsx, struct file_bmap *bmap, void *arg);

int scrub_iterate_filemaps(struct scrub_ctx *ctx, int fd, int whichfork,
		struct file_bmap *key, scrub_bmap_iter_fn fn, void *arg);

#endif /* XFS_SCRUB_FILEMAP_H_ */
