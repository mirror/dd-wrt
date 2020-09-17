// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#ifndef XFS_SCRUB_XFS_SCRUB_H_
#define XFS_SCRUB_XFS_SCRUB_H_

#include "libfrog/fsgeom.h"

extern char *progname;

#define _PATH_PROC_MOUNTS	"/proc/mounts"

extern unsigned int		force_nr_threads;
extern unsigned int		bg_mode;
extern unsigned int		debug;
extern bool			verbose;
extern long			page_size;
extern bool			want_fstrim;
extern bool			stderr_isatty;
extern bool			stdout_isatty;
extern bool			is_service;

enum scrub_mode {
	SCRUB_MODE_DRY_RUN,
	SCRUB_MODE_REPAIR,
};

enum error_action {
	ERRORS_CONTINUE,
	ERRORS_SHUTDOWN,
};

struct scrub_ctx {
	/* Immutable scrub state. */

	/* Strings we need for presentation */
	char			*mntpoint;

	/* Mountpoint info */
	struct stat		mnt_sb;
	struct statvfs		mnt_sv;
	struct statfs		mnt_sf;

	/* Open block devices */
	struct disk		*datadev;
	struct disk		*logdev;
	struct disk		*rtdev;

	/* What does the user want us to do? */
	enum scrub_mode		mode;

	/* How does the user want us to react to errors? */
	enum error_action	error_action;

	/* xfrog context for the mount point */
	struct xfs_fd		mnt;

	/* Number of threads for metadata scrubbing */
	unsigned int		nr_io_threads;

	/* XFS specific geometry */
	struct fs_path		fsinfo;
	void			*fshandle;
	size_t			fshandle_len;

	/* Data block read verification buffer */
	void			*readbuf;

	/* Mutable scrub state; use lock. */
	pthread_mutex_t		lock;
	struct action_list	*action_lists;
	unsigned long long	max_errors;
	unsigned long long	runtime_errors;
	unsigned long long	corruptions_found;
	unsigned long long	unfixable_errors;
	unsigned long long	warnings_found;
	unsigned long long	inodes_checked;
	unsigned long long	bytes_checked;
	unsigned long long	naming_warnings;
	unsigned long long	repairs;
	unsigned long long	preens;
	bool			scrub_setup_succeeded;
	bool			preen_triggers[XFS_SCRUB_TYPE_NR];
};

/* Phase helper functions */
void xfs_shutdown_fs(struct scrub_ctx *ctx);
int scrub_cleanup(struct scrub_ctx *ctx);
int phase1_func(struct scrub_ctx *ctx);
int phase2_func(struct scrub_ctx *ctx);
int phase3_func(struct scrub_ctx *ctx);
int phase4_func(struct scrub_ctx *ctx);
int phase5_func(struct scrub_ctx *ctx);
int phase6_func(struct scrub_ctx *ctx);
int phase7_func(struct scrub_ctx *ctx);

/* Progress estimator functions */
unsigned int scrub_estimate_ag_work(struct scrub_ctx *ctx);
int phase2_estimate(struct scrub_ctx *ctx, uint64_t *items,
		    unsigned int *nr_threads, int *rshift);
int phase3_estimate(struct scrub_ctx *ctx, uint64_t *items,
		    unsigned int *nr_threads, int *rshift);
int phase4_estimate(struct scrub_ctx *ctx, uint64_t *items,
		    unsigned int *nr_threads, int *rshift);
int phase5_estimate(struct scrub_ctx *ctx, uint64_t *items,
		    unsigned int *nr_threads, int *rshift);
int phase6_estimate(struct scrub_ctx *ctx, uint64_t *items,
		    unsigned int *nr_threads, int *rshift);

#endif /* XFS_SCRUB_XFS_SCRUB_H_ */
