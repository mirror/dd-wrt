// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include "libxfs.h"
#include "libxlog.h"

#include "logprint.h"

void
xlog_recover_print_trans_head(
	struct xlog_recover	*tr)
{
	printf(_("TRANS: tid:0x%x  #items:%d  trans:0x%x  q:0x%lx\n"),
	       tr->r_log_tid,
	       tr->r_theader.th_num_items,
	       tr->r_theader.th_tid, (long)&tr->r_itemq);
}

int
xlog_recover_do_trans(
	struct xlog		*log,
	struct xlog_recover	*trans,
	int			pass)
{
	xlog_recover_print_trans(trans, &trans->r_itemq, 3);
	return 0;
}

void
xfs_log_print_trans(
	struct xlog	*log,
	int		print_block_start)
{
	xfs_daddr_t	head_blk, tail_blk;
	int		error;

	error = xlog_find_tail(log, &head_blk, &tail_blk);
	if (error) {
		fprintf(stderr, _("%s: failed to find head and tail, error: %d\n"),
			progname, error);
		exit(1);
	}

	printf(_("    log tail: %lld head: %lld state: %s\n"),
		(long long)tail_blk,
		(long long)head_blk,
		(tail_blk == head_blk)?"<CLEAN>":"<DIRTY>");

	if (print_block_start != -1) {
		printf(_("    override tail: %d\n"), print_block_start);
		tail_blk = print_block_start;
	}
	printf("\n");

	print_record_header = 1;

	if (head_blk == tail_blk)
		return;

	/*
	 * Version 5 superblock log feature mask validation. We know the
	 * log is dirty so check if there are any unknown log features
	 * in what we need to recover. If there are unknown features
	 * (e.g. unsupported transactions) then warn about it.
	 */
	if (XFS_SB_VERSION_NUM(&log->l_mp->m_sb) == XFS_SB_VERSION_5 &&
	    xfs_sb_has_incompat_log_feature(&log->l_mp->m_sb,
				XFS_SB_FEAT_INCOMPAT_LOG_UNKNOWN)) {
		printf(_(
"Superblock has unknown incompatible log features (0x%x) enabled.\n"
"Output may be incomplete or inaccurate. It is recommended that you\n"
"upgrade your xfsprogs installation to match the filesystem features.\n"),
			(log->l_mp->m_sb.sb_features_log_incompat &
				XFS_SB_FEAT_INCOMPAT_LOG_UNKNOWN));
	}

	if ((error = xlog_do_recovery_pass(log, head_blk, tail_blk, XLOG_RECOVER_PASS1))) {
		fprintf(stderr, _("%s: failed in xfs_do_recovery_pass, error: %d\n"),
			progname, error);
		exit(1);
	}
}
