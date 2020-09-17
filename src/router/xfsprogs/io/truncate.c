// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t truncate_cmd;

static int
truncate_f(
	int		argc,
	char		**argv)
{
	off64_t		offset;
	size_t		blocksize, sectsize;

	init_cvtnum(&blocksize, &sectsize);
	offset = cvtnum(blocksize, sectsize, argv[1]);
	if (offset < 0) {
		printf(_("non-numeric truncate argument -- %s\n"), argv[1]);
		exitcode = 1;
		return 0;
	}

	if (ftruncate(file->fd, offset) < 0) {
		perror("ftruncate");
		exitcode = 1;
		return 0;
	}
	return 0;
}

void
truncate_init(void)
{
	truncate_cmd.name = "truncate";
	truncate_cmd.altname = "t";
	truncate_cmd.cfunc = truncate_f;
	truncate_cmd.argmin = 1;
	truncate_cmd.argmax = 1;
	truncate_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	truncate_cmd.args = _("off");
	truncate_cmd.oneline =
		_("truncates the current file at the given offset");

	add_command(&truncate_cmd);
}
