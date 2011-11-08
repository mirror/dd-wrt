/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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
#include "init.h"
#include "io.h"

static cmdinfo_t fsync_cmd;
static cmdinfo_t fdatasync_cmd;

static int
fsync_f(
	int			argc,
	char			**argv)
{
	if (fsync(file->fd) < 0) {
		perror("fsync");
		return 0;
	}
	return 0;
}

static int
fdatasync_f(
	int			argc,
	char			**argv)
{
	if (fdatasync(file->fd) < 0) {
		perror("fdatasync");
		return 0;
	}
	return 0;
}

void
fsync_init(void)
{
	fsync_cmd.name = "fsync";
	fsync_cmd.altname = "s";
	fsync_cmd.cfunc = fsync_f;
	fsync_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fsync_cmd.oneline =
		_("calls fsync(2) to flush all in-core file state to disk");

	fdatasync_cmd.name = "fdatasync";
	fdatasync_cmd.altname = "ds";
	fdatasync_cmd.cfunc = fdatasync_f;
	fdatasync_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	fdatasync_cmd.oneline =
		_("calls fdatasync(2) to flush the files in-core data to disk");

	add_command(&fsync_cmd);
	add_command(&fdatasync_cmd);
}
