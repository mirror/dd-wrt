// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_PROGRESS_H_
#define XFS_SCRUB_PROGRESS_H_

#define CLEAR_EOL	"\033[K"
#define START_IGNORE	'\001'
#define END_IGNORE	'\002'

int progress_init_phase(struct scrub_ctx *ctx, FILE *progress_fp,
			 unsigned int phase, uint64_t max, int rshift,
			 unsigned int nr_threads);
void progress_end_phase(void);
void progress_add(uint64_t x);

#endif /* XFS_SCRUB_PROGRESS_H_ */
