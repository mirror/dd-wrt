/*
 * Copyright (c) 2001-2005 Silicon Graphics, Inc.
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

static cmdinfo_t freeze_cmd;
static cmdinfo_t thaw_cmd;

int
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

int
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
	freeze_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	freeze_cmd.oneline = _("freeze filesystem of current file");

	thaw_cmd.name = "thaw";
	thaw_cmd.cfunc = thaw_f;
	thaw_cmd.argmin = 0;
	thaw_cmd.argmax = 0;
	thaw_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	thaw_cmd.oneline = _("unfreeze filesystem of current file");

	if (expert) {
		add_command(&freeze_cmd);
		add_command(&thaw_cmd);
	}
}
