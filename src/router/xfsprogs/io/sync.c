// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "io.h"

static cmdinfo_t sync_cmd;

static int
sync_f(
	int			argc,
	char			**argv)
{
	/* sync can't fail */
	sync();
	return 0;
}

#ifdef HAVE_SYNCFS
static cmdinfo_t syncfs_cmd;

static int
syncfs_f(
	int			argc,
	char			**argv)
{
	if (syncfs(file->fd) < 0) {
		perror("syncfs");
		exitcode = 1;
	}
	return 0;
}
#endif

void
sync_init(void)
{
	sync_cmd.name = "sync";
	sync_cmd.cfunc = sync_f;
	sync_cmd.flags = CMD_NOMAP_OK | CMD_NOFILE_OK |
			 CMD_FOREIGN_OK | CMD_FLAG_ONESHOT;
	sync_cmd.oneline =
		_("calls sync(2) to flush all in-core filesystem state to disk");

	add_command(&sync_cmd);

#ifdef HAVE_SYNCFS
	syncfs_cmd.name = "syncfs";
	syncfs_cmd.cfunc = syncfs_f;
	syncfs_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	syncfs_cmd.oneline =
		_("calls syncfs(2) to flush all in-core filesystem state to disk");

	add_command(&syncfs_cmd);
#endif
}
