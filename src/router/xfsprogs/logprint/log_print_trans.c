/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "logprint.h"

void
xlog_recover_print_trans_head(
	xlog_recover_t	*tr)
{
	printf(_("TRANS: tid:0x%x  type:%s  #items:%d  trans:0x%x  q:0x%lx\n"),
	       tr->r_log_tid, trans_type[tr->r_theader.th_type],
	       tr->r_theader.th_num_items,
	       tr->r_theader.th_tid, (long)&tr->r_itemq);
}

int
xlog_recover_do_trans(
	xlog_t		*log,
	xlog_recover_t	*trans,
	int		pass)
{
	xlog_recover_print_trans(trans, &trans->r_itemq, 3);
	return 0;
}

void
xfs_log_print_trans(
	xlog_t		*log,
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
	if ((error = xlog_do_recovery_pass(log, head_blk, tail_blk, XLOG_RECOVER_PASS1))) {
		fprintf(stderr, _("%s: failed in xfs_do_recovery_pass, error: %d\n"),
			progname, error);
		exit(1);
	}
}
