// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "bit.h"
#include "bmap.h"
#include "command.h"
#include "type.h"
#include "faddr.h"
#include "fprint.h"
#include "field.h"
#include "inode.h"
#include "io.h"
#include "init.h"
#include "output.h"
#include "dquot.h"

static int	dquot_f(int argc, char **argv);
static void	dquot_help(void);

static const cmdinfo_t	dquot_cmd = {
	"dquot", NULL, dquot_f, 1, 2, 1, N_("[-g|-p|-u] id"),
N_("set current address to a group, project or user quota block for given ID"),
	dquot_help,
};

const field_t	dqblk_hfld[] = {
	{ "", FLDT_DQBLK, OI(0), C1, 0, TYP_NONE },
	{ NULL }
};

#define	DDOFF(f)	bitize(offsetof(struct xfs_dqblk, dd_ ## f))
#define	DDSZC(f)	szcount(struct xfs_dqblk, dd_ ## f)
const field_t	dqblk_flds[] = {
	{ "diskdq", FLDT_DISK_DQUOT, OI(DDOFF(diskdq)), C1, 0, TYP_NONE },
	{ "fill", FLDT_CHARS, OI(DDOFF(fill)), CI(DDSZC(fill)), FLD_SKIPALL,
	  TYP_NONE },
	{ "crc", FLDT_CRC, OI(DDOFF(crc)), C1, 0, TYP_NONE },
	{ "lsn", FLDT_UINT64X, OI(DDOFF(lsn)), C1, 0, TYP_NONE },
	{ "uuid", FLDT_UUID, OI(DDOFF(uuid)), C1, 0, TYP_NONE },
	{ NULL }
};

#define	DOFF(f)		bitize(offsetof(struct xfs_disk_dquot, d_ ## f))
const field_t	disk_dquot_flds[] = {
	{ "magic", FLDT_UINT16X, OI(DOFF(magic)), C1, 0, TYP_NONE },
	{ "version", FLDT_UINT8X, OI(DOFF(version)), C1, 0, TYP_NONE },
	{ "type", FLDT_UINT8X, OI(DOFF(type)), C1, 0, TYP_NONE },
	{ "id", FLDT_DQID, OI(DOFF(id)), C1, 0, TYP_NONE },
	{ "blk_hardlimit", FLDT_QCNT, OI(DOFF(blk_hardlimit)), C1, 0,
	  TYP_NONE },
	{ "blk_softlimit", FLDT_QCNT, OI(DOFF(blk_softlimit)), C1, 0,
	  TYP_NONE },
	{ "ino_hardlimit", FLDT_QCNT, OI(DOFF(ino_hardlimit)), C1, 0,
	  TYP_NONE },
	{ "ino_softlimit", FLDT_QCNT, OI(DOFF(ino_softlimit)), C1, 0,
	  TYP_NONE },
	{ "bcount", FLDT_QCNT, OI(DOFF(bcount)), C1, 0, TYP_NONE },
	{ "icount", FLDT_QCNT, OI(DOFF(icount)), C1, 0, TYP_NONE },
	{ "itimer", FLDT_QTIMER, OI(DOFF(itimer)), C1, 0, TYP_NONE },
	{ "btimer", FLDT_QTIMER, OI(DOFF(btimer)), C1, 0, TYP_NONE },
	{ "iwarns", FLDT_QWARNCNT, OI(DOFF(iwarns)), C1, 0, TYP_NONE },
	{ "bwarns", FLDT_QWARNCNT, OI(DOFF(bwarns)), C1, 0, TYP_NONE },
	{ "pad0", FLDT_UINT32X, OI(DOFF(pad0)), C1, FLD_SKIPALL, TYP_NONE },
	{ "rtb_hardlimit", FLDT_QCNT, OI(DOFF(rtb_hardlimit)), C1, 0,
	  TYP_NONE },
	{ "rtb_softlimit", FLDT_QCNT, OI(DOFF(rtb_softlimit)), C1, 0,
	  TYP_NONE },
	{ "rtbcount", FLDT_QCNT, OI(DOFF(rtbcount)), C1, 0, TYP_NONE },
	{ "rtbtimer", FLDT_QTIMER, OI(DOFF(rtbtimer)), C1, 0, TYP_NONE },
	{ "rtbwarns", FLDT_QWARNCNT, OI(DOFF(rtbwarns)), C1, 0, TYP_NONE },
	{ "pad", FLDT_UINT16X, OI(DOFF(pad)), C1, FLD_SKIPALL, TYP_NONE },
	{ NULL }
};

static void
dquot_help(void)
{
}

static int
dquot_f(
	int		argc,
	char		**argv)
{
	bmap_ext_t	bm;
	int		c;
	int		dogrp;
	int		doprj;
	xfs_dqid_t	id;
	xfs_ino_t	ino;
	xfs_extnum_t	nex;
	char		*p;
	int		perblock;
	xfs_fileoff_t	qbno;
	int		qoff;
	char		*s;

	dogrp = doprj = optind = 0;
	while ((c = getopt(argc, argv, "gpu")) != EOF) {
		switch (c) {
		case 'g':
			dogrp = 1;
			doprj = 0;
			break;
		case 'p':
			doprj = 1;
			dogrp = 0;
			break;
		case 'u':
			dogrp = doprj = 0;
			break;
		default:
			dbprintf(_("bad option for dquot command\n"));
			return 0;
		}
	}
	s = doprj ? _("project") : dogrp ? _("group") : _("user");
	if (optind != argc - 1) {
		dbprintf(_("dquot command requires one %s id argument\n"), s);
		return 0;
	}
	ino = mp->m_sb.sb_uquotino;
	if (doprj)
		ino = mp->m_sb.sb_pquotino;
	else if (dogrp)
		ino = mp->m_sb.sb_gquotino;

	if (ino == 0 || ino == NULLFSINO) {
		dbprintf(_("no %s quota inode present\n"), s);
		return 0;
	}
	id = (xfs_dqid_t)strtol(argv[optind], &p, 0);
	if (*p != '\0') {
		dbprintf(_("bad %s id for dquot %s\n"), s, argv[optind]);
		return 0;
	}
	perblock = (int)(mp->m_sb.sb_blocksize / sizeof(struct xfs_dqblk));
	qbno = (xfs_fileoff_t)id / perblock;
	qoff = (int)(id % perblock);
	push_cur();
	set_cur_inode(ino);
	nex = 1;
	bmap(qbno, 1, XFS_DATA_FORK, &nex, &bm);
	pop_cur();
	if (nex == 0) {
		dbprintf(_("no %s quota data for id %d\n"), s, id);
		return 0;
	}
	set_cur(&typtab[TYP_DQBLK], XFS_FSB_TO_DADDR(mp, bm.startblock), blkbb,
		DB_RING_IGN, NULL);
	iocur_top->dquot_buf = 1;
	off_cur(qoff * (int)sizeof(struct xfs_dqblk), sizeof(struct xfs_dqblk));
	ring_add();
	return 0;
}

void
xfs_dquot_set_crc(
	struct xfs_buf *bp)
{
	ASSERT((iocur_top->dquot_buf));
	ASSERT(iocur_top->bp == bp);

	xfs_update_cksum(iocur_top->data, sizeof(struct xfs_dqblk),
			 XFS_DQUOT_CRC_OFF);
}

void
dquot_init(void)
{
	add_command(&dquot_cmd);
}
