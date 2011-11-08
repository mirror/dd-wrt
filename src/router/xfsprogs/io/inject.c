/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include "init.h"
#include "io.h"

static cmdinfo_t inject_cmd;

static int
error_tag(char *name)
{
	static struct {
		int	tag;
		char	*name;
	} *e, eflags[] = {
#define XFS_ERRTAG_NOERROR                              0
		{ XFS_ERRTAG_NOERROR,			"noerror" },
#define XFS_ERRTAG_IFLUSH_1                             1
		{ XFS_ERRTAG_IFLUSH_1,			"iflush1" },
#define XFS_ERRTAG_IFLUSH_2                             2
		{ XFS_ERRTAG_IFLUSH_2,			"iflush2" },
#define XFS_ERRTAG_IFLUSH_3                             3
		{ XFS_ERRTAG_IFLUSH_3,			"iflush3" },
#define XFS_ERRTAG_IFLUSH_4                             4
		{ XFS_ERRTAG_IFLUSH_4,			"iflush4" },
#define XFS_ERRTAG_IFLUSH_5                             5
		{ XFS_ERRTAG_IFLUSH_5,			"iflush5" },
#define XFS_ERRTAG_IFLUSH_6                             6
		{ XFS_ERRTAG_IFLUSH_6,			"iflush6" },
#define XFS_ERRTAG_DA_READ_BUF                          7
		{ XFS_ERRTAG_DA_READ_BUF,		"dareadbuf" },
#define XFS_ERRTAG_BTREE_CHECK_LBLOCK                   8
		{ XFS_ERRTAG_BTREE_CHECK_LBLOCK,	"btree_chk_lblk" },
#define XFS_ERRTAG_BTREE_CHECK_SBLOCK                   9
		{ XFS_ERRTAG_BTREE_CHECK_SBLOCK,	"btree_chk_sblk" },
#define XFS_ERRTAG_ALLOC_READ_AGF                       10
		{ XFS_ERRTAG_ALLOC_READ_AGF,		"readagf" },
#define XFS_ERRTAG_IALLOC_READ_AGI                      11
		{ XFS_ERRTAG_IALLOC_READ_AGI,		"readagi" },
#define XFS_ERRTAG_ITOBP_INOTOBP                        12
		{ XFS_ERRTAG_ITOBP_INOTOBP,		"itobp" },
#define XFS_ERRTAG_IUNLINK                              13
		{ XFS_ERRTAG_IUNLINK,			"iunlink" },
#define XFS_ERRTAG_IUNLINK_REMOVE                       14
		{ XFS_ERRTAG_IUNLINK_REMOVE,		"iunlinkrm" },
#define XFS_ERRTAG_DIR_INO_VALIDATE                     15
		{ XFS_ERRTAG_DIR_INO_VALIDATE,		"dirinovalid" },
#define XFS_ERRTAG_BULKSTAT_READ_CHUNK                  16
		{ XFS_ERRTAG_BULKSTAT_READ_CHUNK,	"bulkstat" },
#define XFS_ERRTAG_IODONE_IOERR                         17
		{ XFS_ERRTAG_IODONE_IOERR,		"logiodone" },
#define XFS_ERRTAG_STRATREAD_IOERR                      18
		{ XFS_ERRTAG_STRATREAD_IOERR,		"stratread" },
#define XFS_ERRTAG_STRATCMPL_IOERR                      19
		{ XFS_ERRTAG_STRATCMPL_IOERR,		"stratcmpl" },
#define XFS_ERRTAG_DIOWRITE_IOERR                       20
		{ XFS_ERRTAG_DIOWRITE_IOERR,		"diowrite" },
#define XFS_ERRTAG_BMAPIFORMAT                          21
		{ XFS_ERRTAG_BMAPIFORMAT,		"bmapifmt" },
#define XFS_ERRTAG_MAX                                  22
		{ XFS_ERRTAG_MAX,			NULL }
	};
	int	count;

	/* Search for a name */
	if (name) {
		for (e = eflags; e->name; e++)
			if (strcmp(name, e->name) == 0)
				return e->tag;
		return -1;
	}

	/* Dump all the names */
	fputs("tags: [ ", stdout);
	for (count = 0, e = eflags; e->name; e++, count++) {
		if (count) {
			fputs(", ", stdout);
			if (!(count % 5))
				fputs("\n\t", stdout);
		}
		fputs(e->name, stdout);
	}
	fputs(" ]\n", stdout);
	return 0;
}

static void
inject_help(void)
{
	printf(_(
"\n"
" inject errors into the filesystem of the currently open file\n"
"\n"
" Example:\n"
" 'inject readagf' - cause errors on allocation group freespace reads\n"
"\n"
" Causes the kernel to generate and react to errors within XFS, provided\n"
" the XFS kernel code has been built with debugging features enabled.\n"
" With no arguments, displays the list of error injection tags.\n"
"\n"));
}

static int
inject_f(
	int			argc,
	char			**argv)
{
	xfs_error_injection_t	error;
	int			command = XFS_IOC_ERROR_INJECTION;

	if (argc == 1)
		return error_tag(NULL);

	while (--argc > 0) {
		error.fd = file->fd;
		if ((error.errtag = error_tag(argv[argc])) < 0) {
			fprintf(stderr, _("no such tag -- %s\n"), argv[1]);
			continue;
		}
		if (error.errtag == XFS_ERRTAG_NOERROR)
			command = XFS_IOC_ERROR_CLEARALL;
		if ((xfsctl(file->name, file->fd, command, &error)) < 0) {
			perror("XFS_IOC_ERROR_INJECTION");
			continue;
		}
	}
	return 0;
}

void
inject_init(void)
{
	inject_cmd.name = "inject";
	inject_cmd.cfunc = inject_f;
	inject_cmd.argmin = 0;
	inject_cmd.argmax = -1;
	inject_cmd.flags = CMD_NOMAP_OK;
	inject_cmd.args = _("[tag ...]");
	inject_cmd.oneline = _("inject errors into a filesystem");
	inject_cmd.help = inject_help;

	if (expert)
		add_command(&inject_cmd);
}
