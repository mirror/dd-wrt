// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs.h"
#include "command.h"
#include "fsmap.h"
#include "output.h"
#include "init.h"

struct fsmap_info {
	unsigned long long	nr;
	xfs_agnumber_t		agno;
};

static int
fsmap_fn(
	struct xfs_btree_cur	*cur,
	struct xfs_rmap_irec	*rec,
	void			*priv)
{
	struct fsmap_info	*info = priv;

	dbprintf(_("%llu: %u/%u len %u owner %lld offset %llu bmbt %d attrfork %d extflag %d\n"),
		info->nr, info->agno, rec->rm_startblock,
		rec->rm_blockcount, rec->rm_owner, rec->rm_offset,
		!!(rec->rm_flags & XFS_RMAP_BMBT_BLOCK),
		!!(rec->rm_flags & XFS_RMAP_ATTR_FORK),
		!!(rec->rm_flags & XFS_RMAP_UNWRITTEN));
	info->nr++;

	return 0;
}

static void
fsmap(
	xfs_fsblock_t		start_fsb,
	xfs_fsblock_t		end_fsb)
{
	struct fsmap_info	info;
	xfs_agnumber_t		start_ag;
	xfs_agnumber_t		end_ag;
	xfs_agnumber_t		agno;
	xfs_daddr_t		eofs;
	struct xfs_rmap_irec	low = {0};
	struct xfs_rmap_irec	high = {0};
	struct xfs_btree_cur	*bt_cur;
	struct xfs_buf		*agbp;
	int			error;

	eofs = XFS_FSB_TO_BB(mp, mp->m_sb.sb_dblocks);
	if (XFS_FSB_TO_DADDR(mp, end_fsb) >= eofs)
		end_fsb = XFS_DADDR_TO_FSB(mp, eofs - 1);

	low.rm_startblock = XFS_FSB_TO_AGBNO(mp, start_fsb);
	high.rm_startblock = -1U;
	high.rm_owner = ULLONG_MAX;
	high.rm_offset = ULLONG_MAX;
	high.rm_flags = XFS_RMAP_ATTR_FORK | XFS_RMAP_BMBT_BLOCK | XFS_RMAP_UNWRITTEN;

	start_ag = XFS_FSB_TO_AGNO(mp, start_fsb);
	end_ag = XFS_FSB_TO_AGNO(mp, end_fsb);

	info.nr = 0;
	for (agno = start_ag; agno <= end_ag; agno++) {
		if (agno == end_ag)
			high.rm_startblock = XFS_FSB_TO_AGBNO(mp, end_fsb);

		error = -libxfs_alloc_read_agf(mp, NULL, agno, 0, &agbp);
		if (error) {
			dbprintf(_("Error %d while reading AGF.\n"), error);
			return;
		}

		bt_cur = libxfs_rmapbt_init_cursor(mp, NULL, agbp, agno);
		if (!bt_cur) {
			libxfs_buf_relse(agbp);
			dbprintf(_("Not enough memory.\n"));
			return;
		}

		info.agno = agno;
		error = -libxfs_rmap_query_range(bt_cur, &low, &high,
				fsmap_fn, &info);
		if (error) {
			libxfs_btree_del_cursor(bt_cur, XFS_BTREE_ERROR);
			libxfs_buf_relse(agbp);
			dbprintf(_("Error %d while querying fsmap btree.\n"),
				error);
			return;
		}

		libxfs_btree_del_cursor(bt_cur, XFS_BTREE_NOERROR);
		libxfs_buf_relse(agbp);

		if (agno == start_ag)
			low.rm_startblock = 0;
	}
}

static int
fsmap_f(
	int			argc,
	char			**argv)
{
	char			*p;
	int			c;
	xfs_fsblock_t		start_fsb = 0;
	xfs_fsblock_t		end_fsb = NULLFSBLOCK;

	if (!xfs_sb_version_hasrmapbt(&mp->m_sb)) {
		dbprintf(_("Filesystem does not support reverse mapping btree.\n"));
		return 0;
	}

	while ((c = getopt(argc, argv, "")) != EOF) {
		switch (c) {
		default:
			dbprintf(_("Bad option for fsmap command.\n"));
			return 0;
		}
	}

	if (argc > optind) {
		start_fsb = strtoull(argv[optind], &p, 0);
		if (*p != '\0' || start_fsb >= mp->m_sb.sb_dblocks) {
			dbprintf(_("Bad fsmap start_fsb %s.\n"), argv[optind]);
			return 0;
		}
	}

	if (argc > optind + 1) {
		end_fsb = strtoull(argv[optind + 1], &p, 0);
		if (*p != '\0') {
			dbprintf(_("Bad fsmap end_fsb %s.\n"), argv[optind + 1]);
			return 0;
		}
	}

	fsmap(start_fsb, end_fsb);

	return 0;
}

static const cmdinfo_t	fsmap_cmd =
	{ "fsmap", NULL, fsmap_f, 0, 2, 0,
	  N_("[start_fsb] [end_fsb]"),
	  N_("display reverse mapping(s)"), NULL };

void
fsmap_init(void)
{
	add_command(&fsmap_cmd);
}
