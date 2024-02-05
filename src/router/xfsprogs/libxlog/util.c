// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
#include "libxlog.h"

int print_exit;
int print_skip_uuid;
int print_record_header;
libxfs_init_t x;

/*
 * Return 1 for dirty, 0 for clean, -1 for errors
 */
int
xlog_is_dirty(
	struct xfs_mount	*mp,
	struct xlog		*log,
	libxfs_init_t		*x,
	int			verbose)
{
	int			error;
	xfs_daddr_t		head_blk, tail_blk;

	memset(log, 0, sizeof(*log));

	/* We (re-)init members of libxfs_init_t here?  really? */
	x->logBBsize = XFS_FSB_TO_BB(mp, mp->m_sb.sb_logblocks);
	x->logBBstart = XFS_FSB_TO_DADDR(mp, mp->m_sb.sb_logstart);
	x->lbsize = BBSIZE;
	if (xfs_has_sector(mp))
		x->lbsize <<= (mp->m_sb.sb_logsectlog - BBSHIFT);

	log->l_dev = mp->m_logdev_targp;
	log->l_logBBsize = x->logBBsize;
	log->l_logBBstart = x->logBBstart;
	log->l_sectBBsize = BTOBB(x->lbsize);
	log->l_mp = mp;
	if (xfs_has_sector(mp)) {
		log->l_sectbb_log = mp->m_sb.sb_logsectlog - BBSHIFT;
		ASSERT(log->l_sectbb_log <= mp->m_sectbb_log);
		/* for larger sector sizes, must have v2 or external log */
		ASSERT(log->l_sectbb_log == 0 ||
			log->l_logBBstart == 0 ||
			xfs_has_logv2(mp));
		ASSERT(mp->m_sb.sb_logsectlog >= BBSHIFT);
	}
	log->l_sectbb_mask = (1 << log->l_sectbb_log) - 1;

	error = xlog_find_tail(log, &head_blk, &tail_blk);
	if (error) {
		xlog_warn(_("%s: cannot find log head/tail "
			  "(xlog_find_tail=%d)\n"),
			__func__, error);
		return -1;
	}

	if (verbose)
		xlog_warn(
	_("%s: head block %" PRId64 " tail block %" PRId64 "\n"),
			__func__, head_blk, tail_blk);

	if (head_blk != tail_blk)
		return 1;

	return 0;
}

static int
header_check_uuid(xfs_mount_t *mp, xlog_rec_header_t *head)
{
    char uu_log[64], uu_sb[64];

    if (print_skip_uuid)
		return 0;
    if (!platform_uuid_compare(&mp->m_sb.sb_uuid, &head->h_fs_uuid))
		return 0;

    platform_uuid_unparse(&mp->m_sb.sb_uuid, uu_sb);
    platform_uuid_unparse(&head->h_fs_uuid, uu_log);

    printf(_("* ERROR: mismatched uuid in log\n"
	     "*            SB : %s\n*            log: %s\n"),
	    uu_sb, uu_log);

    memcpy(&mp->m_sb.sb_uuid, &head->h_fs_uuid, sizeof(uuid_t));

    return 0;
}

int
xlog_header_check_recover(xfs_mount_t *mp, xlog_rec_header_t *head)
{
    if (print_record_header)
	printf(_("\nLOG REC AT LSN cycle %d block %d (0x%x, 0x%x)\n"),
	       CYCLE_LSN(be64_to_cpu(head->h_lsn)),
	       BLOCK_LSN(be64_to_cpu(head->h_lsn)),
	       CYCLE_LSN(be64_to_cpu(head->h_lsn)),
	       BLOCK_LSN(be64_to_cpu(head->h_lsn)));

    if (be32_to_cpu(head->h_magicno) != XLOG_HEADER_MAGIC_NUM) {

	printf(_("* ERROR: bad magic number in log header: 0x%x\n"),
		be32_to_cpu(head->h_magicno));

    } else if (header_check_uuid(mp, head)) {

	/* failed - fall through */

    } else if (be32_to_cpu(head->h_fmt) != XLOG_FMT) {

	printf(_("* ERROR: log format incompatible (log=%d, ours=%d)\n"),
		be32_to_cpu(head->h_fmt), XLOG_FMT);

    } else {
	/* everything is ok */
	return 0;
    }

    /* bail out now or just carry on regardless */
    if (print_exit)
	xlog_exit(_("Bad log"));

    return 0;
}

int
xlog_header_check_mount(xfs_mount_t *mp, xlog_rec_header_t *head)
{
    if (platform_uuid_is_null(&head->h_fs_uuid)) return 0;
    if (header_check_uuid(mp, head)) {
	/* bail out now or just carry on regardless */
	if (print_exit)
	    xlog_exit(_("Bad log"));
    }
    return 0;
}

/*
 * Userspace versions of common diagnostic routines (varargs fun).
 */
void
xlog_warn(char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	va_end(ap);
}

void
xlog_exit(char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	va_end(ap);
	exit(1);
}

void
xlog_panic(char *fmt, ...)
{
	va_list	ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	va_end(ap);
	abort();
}
