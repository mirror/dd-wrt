// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <stdint.h>
#include <stdlib.h>
#include <sys/statvfs.h>
#include <linux/fsmap.h>
#include "libfrog/paths.h"
#include "libfrog/ptvar.h"
#include "list.h"
#include "xfs_scrub.h"
#include "common.h"
#include "scrub.h"
#include "fscounters.h"
#include "spacemap.h"
#include "repair.h"

/* Phase 7: Check summary counters. */

struct summary_counts {
	unsigned long long	dbytes;		/* data dev bytes */
	unsigned long long	rbytes;		/* rt dev bytes */
	unsigned long long	next_phys;	/* next phys bytes we see? */
	unsigned long long	agbytes;	/* freespace bytes */
};

/* Record block usage. */
static int
count_block_summary(
	struct scrub_ctx	*ctx,
	struct fsmap		*fsmap,
	void			*arg)
{
	struct summary_counts	*counts;
	unsigned long long	len;
	int			ret;

	counts = ptvar_get((struct ptvar *)arg, &ret);
	if (ret) {
		str_liberror(ctx, -ret, _("retrieving summary counts"));
		return -ret;
	}
	if (fsmap->fmr_device == ctx->fsinfo.fs_logdev)
		return 0;
	if ((fsmap->fmr_flags & FMR_OF_SPECIAL_OWNER) &&
	    fsmap->fmr_owner == XFS_FMR_OWN_FREE)
		return 0;

	len = fsmap->fmr_length;

	/* freesp btrees live in free space, need to adjust counters later. */
	if ((fsmap->fmr_flags & FMR_OF_SPECIAL_OWNER) &&
	    fsmap->fmr_owner == XFS_FMR_OWN_AG) {
		counts->agbytes += fsmap->fmr_length;
	}
	if (fsmap->fmr_device == ctx->fsinfo.fs_rtdev) {
		/* Count realtime extents. */
		counts->rbytes += len;
	} else {
		/* Count datadev extents. */
		if (counts->next_phys >= fsmap->fmr_physical + len)
			return 0;
		else if (counts->next_phys > fsmap->fmr_physical)
			len = counts->next_phys - fsmap->fmr_physical;
		counts->dbytes += len;
		counts->next_phys = fsmap->fmr_physical + fsmap->fmr_length;
	}

	return 0;
}

/* Add all the summaries in the per-thread counter */
static int
add_summaries(
	struct ptvar		*ptv,
	void			*data,
	void			*arg)
{
	struct summary_counts	*total = arg;
	struct summary_counts	*item = data;

	total->dbytes += item->dbytes;
	total->rbytes += item->rbytes;
	total->agbytes += item->agbytes;
	return 0;
}

/*
 * Count all inodes and blocks in the filesystem as told by GETFSMAP and
 * BULKSTAT, and compare that to summary counters.  Since this is a live
 * filesystem we'll be content if the summary counts are within 10% of
 * what we observed.
 */
int
phase7_func(
	struct scrub_ctx	*ctx)
{
	struct summary_counts	totalcount = {0};
	struct action_list	alist;
	struct ptvar		*ptvar;
	unsigned long long	used_data;
	unsigned long long	used_rt;
	unsigned long long	used_files;
	unsigned long long	stat_data;
	unsigned long long	stat_rt;
	uint64_t		counted_inodes = 0;
	unsigned long long	absdiff;
	unsigned long long	d_blocks;
	unsigned long long	d_bfree;
	unsigned long long	r_blocks;
	unsigned long long	r_bfree;
	bool			complain;
	int			ip;
	int			error;

	/* Check and fix the fs summary counters. */
	action_list_init(&alist);
	error = scrub_fs_summary(ctx, &alist);
	if (error)
		return error;
	error = action_list_process(ctx, -1, &alist,
			ALP_COMPLAIN_IF_UNFIXED | ALP_NOPROGRESS);
	if (error)
		return error;

	/* Flush everything out to disk before we start counting. */
	error = syncfs(ctx->mnt.fd);
	if (error) {
		str_errno(ctx, ctx->mntpoint);
		return error;
	}

	error = -ptvar_alloc(scrub_nproc(ctx), sizeof(struct summary_counts),
			&ptvar);
	if (error) {
		str_liberror(ctx, error, _("setting up block counter"));
		return error;
	}

	/* Use fsmap to count blocks. */
	error = scrub_scan_all_spacemaps(ctx, count_block_summary, ptvar);
	if (error)
		goto out_free;
	error = -ptvar_foreach(ptvar, add_summaries, &totalcount);
	if (error) {
		str_liberror(ctx, error, _("counting blocks"));
		goto out_free;
	}
	ptvar_free(ptvar);

	/* Scan the whole fs. */
	error = scrub_count_all_inodes(ctx, &counted_inodes);
	if (error) {
		str_liberror(ctx, error, _("counting inodes"));
		return error;
	}

	error = scrub_scan_estimate_blocks(ctx, &d_blocks, &d_bfree, &r_blocks,
			&r_bfree, &used_files);
	if (error) {
		str_liberror(ctx, error, _("estimating verify work"));
		return error;
	}

	/*
	 * If we counted blocks with fsmap, then dblocks includes
	 * blocks for the AGFL and the freespace/rmap btrees.  The
	 * filesystem treats them as "free", but since we scanned
	 * them, we'll consider them used.
	 */
	d_bfree -= cvt_b_to_off_fsbt(&ctx->mnt, totalcount.agbytes);

	/* Report on what we found. */
	used_data = cvt_off_fsb_to_b(&ctx->mnt, d_blocks - d_bfree);
	used_rt = cvt_off_fsb_to_b(&ctx->mnt, r_blocks - r_bfree);
	stat_data = totalcount.dbytes;
	stat_rt = totalcount.rbytes;

	/*
	 * Complain if the counts are off by more than 10% unless
	 * the inaccuracy is less than 32MB worth of blocks or 100 inodes.
	 */
	absdiff = 1ULL << 25;
	complain = verbose;
	complain |= !within_range(ctx, stat_data, used_data, absdiff, 1, 10,
			_("data blocks"));
	complain |= !within_range(ctx, stat_rt, used_rt, absdiff, 1, 10,
			_("realtime blocks"));
	complain |= !within_range(ctx, counted_inodes, used_files, 100, 1, 10,
			_("inodes"));

	if (complain) {
		double		d, r, i;
		char		*du, *ru, *iu;

		if (used_rt || stat_rt) {
			d = auto_space_units(used_data, &du);
			r = auto_space_units(used_rt, &ru);
			i = auto_units(used_files, &iu, &ip);
			fprintf(stdout,
_("%.1f%s data used;  %.1f%s realtime data used;  %.*f%s inodes used.\n"),
					d, du, r, ru, ip, i, iu);
			d = auto_space_units(stat_data, &du);
			r = auto_space_units(stat_rt, &ru);
			i = auto_units(counted_inodes, &iu, &ip);
			fprintf(stdout,
_("%.1f%s data found; %.1f%s realtime data found; %.*f%s inodes found.\n"),
					d, du, r, ru, ip, i, iu);
		} else {
			d = auto_space_units(used_data, &du);
			i = auto_units(used_files, &iu, &ip);
			fprintf(stdout,
_("%.1f%s data used;  %.*f%s inodes used.\n"),
					d, du, ip, i, iu);
			d = auto_space_units(stat_data, &du);
			i = auto_units(counted_inodes, &iu, &ip);
			fprintf(stdout,
_("%.1f%s data found; %.*f%s inodes found.\n"),
					d, du, ip, i, iu);
		}
		fflush(stdout);
	}

	/*
	 * Complain if the checked inode counts are off, which
	 * implies an incomplete check.
	 */
	if (verbose ||
	    !within_range(ctx, counted_inodes, ctx->inodes_checked, 100, 1, 10,
			_("checked inodes"))) {
		double		i1, i2;
		char		*i1u, *i2u;
		int		i1p, i2p;

		i1 = auto_units(counted_inodes, &i1u, &i1p);
		i2 = auto_units(ctx->inodes_checked, &i2u, &i2p);
		fprintf(stdout,
_("%.*f%s inodes counted; %.*f%s inodes checked.\n"),
				i1p, i1, i1u, i2p, i2, i2u);
		fflush(stdout);
	}

	/*
	 * Complain if the checked block counts are off, which
	 * implies an incomplete check.
	 */
	if (ctx->bytes_checked &&
	    (verbose ||
	     !within_range(ctx, used_data + used_rt,
			ctx->bytes_checked, absdiff, 1, 10,
			_("verified blocks")))) {
		double		b1, b2;
		char		*b1u, *b2u;

		b1 = auto_space_units(used_data + used_rt, &b1u);
		b2 = auto_space_units(ctx->bytes_checked, &b2u);
		fprintf(stdout,
_("%.1f%s data counted; %.1f%s data verified.\n"),
				b1, b1u, b2, b2u);
		fflush(stdout);
	}

	return 0;
out_free:
	ptvar_free(ptvar);
	return error;
}
