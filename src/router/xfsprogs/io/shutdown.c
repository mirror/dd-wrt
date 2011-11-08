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
			return command_usage(&shutdown_cmd);
		}
	}

	if ((xfsctl(file->name, file->fd, XFS_IOC_GOINGDOWN, &flag)) < 0) {
		perror("XFS_IOC_GOINGDOWN");
		return 0;
	}
	return 0;
}

void
shutdown_init(void)
{
	shutdown_cmd.name = "shutdown";
	shutdown_cmd.cfunc = shutdown_f;
	shutdown_cmd.argmin = 0;
	shutdown_cmd.argmax = 1;
	shutdown_cmd.flags = CMD_NOMAP_OK;
	shutdown_cmd.args = _("[-f]");
	shutdown_cmd.oneline =
		_("shuts down the filesystem where the current file resides");

	if (expert)
		add_command(&shutdown_cmd);
}
