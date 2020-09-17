// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs.h"
#include "command.h"
#include "init.h"
#include "libfrog/paths.h"
#include "libfrog/fsgeom.h"
#include "space.h"

static void
info_help(void)
{
	printf(_(
"\n"
" Pretty-prints the filesystem geometry as derived from the superblock.\n"
" The output has the same format as mkfs.xfs, xfs_info, and other utilities.\n"
" The opened file must be an XFS mount point.\n"
"\n"
));

}

static int
info_f(
	int			argc,
	char			**argv)
{
	if (fs_table_lookup_mount(file->name) == NULL) {
		fprintf(stderr, _("%s: Not a XFS mount point.\n"), file->name);
		return 1;
	}

	xfs_report_geom(&file->xfd.fsgeom, file->fs_path.fs_name,
			file->fs_path.fs_log, file->fs_path.fs_rt);
	return 0;
}

static const struct cmdinfo info_cmd = {
	.name =		"info",
	.altname =	"i",
	.cfunc =	info_f,
	.argmin =	0,
	.argmax =	0,
	.canpush =	0,
	.args =		NULL,
	.flags =	CMD_FLAG_ONESHOT,
	.oneline =	N_("pretty-print superblock geometry info"),
	.help =		info_help,
};

void
info_init(void)
{
	add_command(&info_cmd);
}
