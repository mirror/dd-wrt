// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t resblks_cmd;
static int resblks_f(int argc, char **argv);

static int
resblks_f(
	int			argc,
	char			**argv)
{
	xfs_fsop_resblks_t	res;
	long long		blks;

	if (argc == 2) {
		blks = cvtnum(file->geom.blocksize, file->geom.sectsize, argv[1]);
		if (blks < 0) {
			printf(_("non-numeric argument -- %s\n"), argv[1]);
			exitcode = 1;
			return 0;
		}
		res.resblks = blks;
		if (xfsctl(file->name, file->fd, XFS_IOC_SET_RESBLKS, &res) < 0) {
			perror("XFS_IOC_SET_RESBLKS");
			exitcode = 1;
			return 0;
		}
	} else if (xfsctl(file->name, file->fd, XFS_IOC_GET_RESBLKS, &res) < 0) {
		perror("XFS_IOC_GET_RESBLKS");
		exitcode = 1;
		return 0;
	}
	printf(_("reserved blocks = %llu\n"),
			(unsigned long long) res.resblks);
	printf(_("available reserved blocks = %llu\n"),
			(unsigned long long) res.resblks_avail);
	return 0;
}

void
resblks_init(void)
{
	resblks_cmd.name = "resblks";
	resblks_cmd.cfunc = resblks_f;
	resblks_cmd.argmin = 0;
	resblks_cmd.argmax = 1;
	resblks_cmd.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT;
	resblks_cmd.args = _("[blocks]");
	resblks_cmd.oneline =
		_("get and/or set count of reserved filesystem blocks");

	if (expert)
		add_command(&resblks_cmd);
}
