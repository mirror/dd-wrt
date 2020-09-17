// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2001-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t freeze_cmd;
static cmdinfo_t thaw_cmd;

static int
freeze_f(
	int		argc,
	char		**argv)
{
	int		level = 1;

	if (xfsctl(file->name, file->fd, XFS_IOC_FREEZE, &level) < 0) {
		fprintf(stderr,
			_("%s: cannot freeze filesystem at %s: %s\n"),
			progname, file->name, strerror(errno));
		exitcode = 1;
		return 0;
	}
	return 0;
}

static int
thaw_f(
	int		argc,
	char		**argv)
{
	int		level = 1;

	if (xfsctl(file->name, file->fd, XFS_IOC_THAW, &level) < 0) {
		fprintf(stderr,
			_("%s: cannot unfreeze filesystem mounted at %s: %s\n"),
			progname, file->name, strerror(errno));
		exitcode = 1;
		return 0;
	}
	return 0;
}

void
freeze_init(void)
{
	freeze_cmd.name = "freeze";
	freeze_cmd.cfunc = freeze_f;
	freeze_cmd.argmin = 0;
	freeze_cmd.argmax = 0;
	freeze_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	freeze_cmd.oneline = _("freeze filesystem of current file");

	thaw_cmd.name = "thaw";
	thaw_cmd.cfunc = thaw_f;
	thaw_cmd.argmin = 0;
	thaw_cmd.argmax = 0;
	thaw_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	thaw_cmd.oneline = _("unfreeze filesystem of current file");

	if (expert) {
		add_command(&freeze_cmd);
		add_command(&thaw_cmd);
	}
}
