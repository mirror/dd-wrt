// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Christoph Hellwig.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

#ifndef AT_EMPTY_PATH
#define AT_EMPTY_PATH	0x1000
#endif

static cmdinfo_t flink_cmd;

static void
flink_help(void)
{
	printf(_(
"\n"
"link the open file descriptor to the supplied filename\n"
"\n"
"\n"));
}

static int
flink_f(
	int		argc,
	char		**argv)
{
	if (argc != 2)
		return command_usage(&flink_cmd);

	if (linkat(file->fd, "", AT_FDCWD, argv[1], AT_EMPTY_PATH) < 0) {
		perror("flink");
		exitcode = 1;
		return 0;
	}
	return 0;
}

void
flink_init(void)
{
	flink_cmd.name = "flink";
	flink_cmd.cfunc = flink_f;
	flink_cmd.argmin = 1;
	flink_cmd.argmax = 1;
	flink_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	flink_cmd.args = _("filename");
	flink_cmd.oneline =
		_("link the open file descriptor to the supplied filename");
	flink_cmd.help = flink_help;

	add_command(&flink_cmd);
}
