/*
 * Copyright (c) 2000-2001,2004-2005 Silicon Graphics, Inc.
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

#include <xfs/libxlog.h>

int print_exit;
int print_skip_uuid;
int print_record_header;
libxfs_init_t x;

static int
header_check_uuid(xfs_mount_t *mp, xlog_rec_header_t *head)
{
    char uu_log[64], uu_sb[64];

    if (print_skip_uuid) return 0;
    if (!platform_uuid_compare(&mp->m_sb.sb_uuid, &head->h_fs_uuid)) return 0;

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
