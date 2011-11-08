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
#include <xfs/input.h>
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
		return 0;
	}

	if (ftruncate64(file->fd, offset) < 0) {
		perror("ftruncate");
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
