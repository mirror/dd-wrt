// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2015 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "command.h"
#include "init.h"
#include "output.h"
#include "libxlog.h"
#include "logformat.h"

#define MAX_LSUNIT	256 * 1024	/* max log buf. size */

static int
logformat_f(int argc, char **argv)
{
	xfs_daddr_t	head_blk;
	xfs_daddr_t	tail_blk;
	int		logversion;
	int		lsunit = -1;
	int		cycle = -1;
	int		error;
	int		c;

	logversion = xfs_sb_version_haslogv2(&mp->m_sb) ? 2 : 1;

	while ((c = getopt(argc, argv, "c:s:")) != EOF) {
		switch (c) {
		case 'c':
			cycle = strtol(optarg, NULL, 0);
			if (cycle == 0) {
				dbprintf("invalid cycle\n");
				return -1;
			}
			break;
		case 's':
			lsunit = strtol(optarg, NULL, 0);
			/*
			 * The log stripe unit must be block aligned and no
			 * larger than 256k.
			 */
			if (lsunit > 1 &&
			    (lsunit % mp->m_sb.sb_blocksize ||
			    (logversion == 2 && lsunit > MAX_LSUNIT))) {
				dbprintf("invalid log stripe unit\n");
				return -1;
			}
			break;
		default:
			dbprintf("invalid option\n");
			return -1;
		}
	}

	/*
	 * Check whether the log is dirty. This also determines the current log
	 * cycle if we have to use it by default below.
	 */
	memset(mp->m_log, 0, sizeof(struct xlog));
	mp->m_log->l_mp = mp;
	mp->m_log->l_dev = mp->m_logdev_targp;
	mp->m_log->l_logBBsize = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);
	mp->m_log->l_logBBstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart);
	mp->m_log->l_sectBBsize = BBSIZE;
	if (xfs_sb_version_hassector(&mp->m_sb))
		mp->m_log->l_sectBBsize <<= (mp->m_sb.sb_logsectlog - BBSHIFT);
	mp->m_log->l_sectBBsize = BTOBB(mp->m_log->l_sectBBsize);

	error = xlog_find_tail(mp->m_log, &head_blk, &tail_blk);
	if (error) {
		dbprintf("could not find log head/tail\n");
		return -1;
	}
	if (head_blk != tail_blk) {
		dbprintf(_(
			"The log is dirty. Please mount to replay the log.\n"));
		return -1;
	}

	/*
	 * Use the current cycle and/or log stripe unit if either is not
	 * provided by the user.
	 */
	if (cycle < 0)
		cycle = mp->m_log->l_curr_cycle;
	if (lsunit < 0)
		lsunit = mp->m_sb.sb_logsunit;

	dbprintf("Formatting the log to cycle %d, stripe unit %d bytes.\n",
		 cycle, lsunit);
	error = -libxfs_log_clear(mp->m_logdev_targp, NULL,
				 mp->m_log->l_logBBstart,
				 mp->m_log->l_logBBsize,
				 &mp->m_sb.sb_uuid, logversion, lsunit,
				 XLOG_FMT, cycle, false);
	if (error) {
		dbprintf("error formatting log - %d\n", error);
		return error;
	}

	return 0;
}

static void
logformat_help(void)
{
	dbprintf(_(
"\n"
" The 'logformat' command reformats (clears) the log to the specified log\n"
" cycle and log stripe unit. If the log cycle is not specified, the log is\n"
" reformatted to the current cycle. If the log stripe unit is not specified,\n"
" the stripe unit from the filesystem superblock is used.\n"
"\n"
	));
}

static const struct cmdinfo logformat_cmd = {
	.name =		"logformat",
	.altname =	NULL,
	.cfunc =	logformat_f,
	.argmin =	0,
	.argmax =	4,
	.canpush =	0,
	.args =		N_("[-c cycle] [-s sunit]"),
	.oneline =	N_("reformat the log"),
	.help =		logformat_help,
};

void
logformat_init(void)
{
	if (!expert_mode)
		return;

	add_command(&logformat_cmd);
}

static void
print_logres(
	int			i,
	struct xfs_trans_res	*res)
{
	dbprintf(_("type %d logres %u logcount %d flags 0x%x\n"),
		i, res->tr_logres, res->tr_logcount, res->tr_logflags);
}

static int
logres_f(
	int			argc,
	char			**argv)
{
	struct xfs_trans_res	resv;
	struct xfs_trans_res	*res;
	struct xfs_trans_res	*end_res;
	int			i;

	res = (struct xfs_trans_res *)M_RES(mp);
	end_res = (struct xfs_trans_res *)(M_RES(mp) + 1);
	for (i = 0; res < end_res; i++, res++)
		print_logres(i, res);
	libxfs_log_get_max_trans_res(mp, &resv);
	print_logres(-1, &resv);

	return 0;
}

static void
logres_help(void)
{
	dbprintf(_(
"\n"
" The 'logres' command prints information about all log reservation types.\n"
" This includes the reservation space, the intended transaction roll count,\n"
" and the reservation flags, if any.\n"
"\n"
	));
}

static const struct cmdinfo logres_cmd = {
	.name =		"logres",
	.altname =	NULL,
	.cfunc =	logres_f,
	.argmin =	0,
	.argmax =	0,
	.canpush =	0,
	.args =		NULL,
	.oneline =	N_("dump log reservations"),
	.help =		logres_help,
};

void
logres_init(void)
{
	add_command(&logres_cmd);
}
