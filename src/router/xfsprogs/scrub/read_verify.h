// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_READ_VERIFY_H_
#define XFS_SCRUB_READ_VERIFY_H_

struct scrub_ctx;
struct read_verify_pool;
struct disk;

/* Function called when an IO error happens. */
typedef void (*read_verify_ioerr_fn_t)(struct scrub_ctx *ctx,
		struct disk *disk, uint64_t start, uint64_t length,
		int error, void *arg);

int read_verify_pool_alloc(struct scrub_ctx *ctx, struct disk *disk,
		size_t miniosz, read_verify_ioerr_fn_t ioerr_fn,
		unsigned int submitter_threads,
		struct read_verify_pool **prvp);
void read_verify_pool_abort(struct read_verify_pool *rvp);
int read_verify_pool_flush(struct read_verify_pool *rvp);
void read_verify_pool_destroy(struct read_verify_pool *rvp);

int read_verify_schedule_io(struct read_verify_pool *rvp, uint64_t start,
		uint64_t length, void *end_arg);
int read_verify_force_io(struct read_verify_pool *rvp);
int read_verify_bytes(struct read_verify_pool *rvp, uint64_t *bytes);

#endif /* XFS_SCRUB_READ_VERIFY_H_ */
