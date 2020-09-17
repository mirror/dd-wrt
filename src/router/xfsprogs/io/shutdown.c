// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t shutdown_cmd;

static int
shutdown_f(
	int		argc,
	char		**argv)
{
	int		c, flag = XFS_FSOP_GOING_FLAGS_NOLOGFLUSH;

	while ((c = getopt(argc, argv, "fv")) != -1) {
		switch (c) {
		case 'f':
			flag = XFS_FSOP_GOING_FLAGS_LOGFLUSH;
			break;
		default:
			exitcode = 1;
			return command_usage(&shutdown_cmd);
		}
	}

	if ((xfsctl(file->name, file->fd, XFS_IOC_GOINGDOWN, &flag)) < 0) {
		perror("XFS_IOC_GOINGDOWN");
		exitcode = 1;
		return 0;
	}
	return 0;
}

static void
shutdown_help(void)
{
	printf(_(
"\n"
" Shuts down the filesystem and prevents any further IO from occurring.\n"
"\n"
" By default, shutdown will not flush completed transactions to disk\n"
" before shutting the filesystem down, simulating a disk failure or crash.\n"
" With -f, the log will be flushed to disk, matching XFS behavior when\n"
" metadata corruption is encountered.\n"
"\n"
" -f -- Flush completed transactions to disk before shut down.\n"
"\n"));
}

void
shutdown_init(void)
{
	shutdown_cmd.name = "shutdown";
	shutdown_cmd.cfunc = shutdown_f;
	shutdown_cmd.argmin = 0;
	shutdown_cmd.argmax = 1;
	shutdown_cmd.flags = CMD_NOMAP_OK | CMD_FLAG_ONESHOT | CMD_FLAG_FOREIGN_OK;
	shutdown_cmd.args = _("[-f]");
	shutdown_cmd.help = shutdown_help;
	shutdown_cmd.oneline =
		_("shuts down the filesystem where the current file resides");

	if (expert)
		add_command(&shutdown_cmd);
}
