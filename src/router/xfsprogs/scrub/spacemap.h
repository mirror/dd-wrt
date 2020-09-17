// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_SPACEMAP_H_
#define XFS_SCRUB_SPACEMAP_H_

/*
 * Visit each space mapping in the filesystem.  Return 0 to continue iteration
 * or a positive error code to stop iterating and return to the caller.
 */
typedef int (*scrub_fsmap_iter_fn)(struct scrub_ctx *ctx,
		struct fsmap *fsr, void *arg);

int scrub_iterate_fsmap(struct scrub_ctx *ctx, struct fsmap *keys,
		scrub_fsmap_iter_fn fn, void *arg);
int scrub_scan_all_spacemaps(struct scrub_ctx *ctx, scrub_fsmap_iter_fn fn,
		void *arg);

#endif /* XFS_SCRUB_SPACEMAP_H_ */
