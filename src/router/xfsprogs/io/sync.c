/*
 * Copyright (c) 2014 Red Hat, Inc.
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
	/* syncfs can't fail */
	syncfs(file->fd);
	return 0;
}
#endif

void
sync_init(void)
{
	sync_cmd.name = "sync";
	sync_cmd.cfunc = sync_f;
	sync_cmd.flags = CMD_NOMAP_OK | CMD_NOFILE_OK | CMD_FOREIGN_OK;
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
