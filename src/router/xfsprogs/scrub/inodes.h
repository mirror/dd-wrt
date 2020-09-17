// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_INODES_H_
#define XFS_SCRUB_INODES_H_

/*
 * Visit each space mapping of an inode fork.  Return 0 to continue iteration
 * or a positive error code to interrupt iteraton.  If ESTALE is returned,
 * iteration will be restarted from the beginning of the inode allocation
 * group.  Any other non zero value will stop iteration.  The special return
 * value ECANCELED can be used to stop iteration, because the inode iteration
 * function never generates that error code on its own.
 */
typedef int (*scrub_inode_iter_fn)(struct scrub_ctx *ctx,
		struct xfs_handle *handle, struct xfs_bulkstat *bs, void *arg);

int scrub_scan_all_inodes(struct scrub_ctx *ctx, scrub_inode_iter_fn fn,
		void *arg);

int scrub_open_handle(struct xfs_handle *handle);

#endif /* XFS_SCRUB_INODES_H_ */
