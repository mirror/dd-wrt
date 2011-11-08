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

static cmdinfo_t resblks_cmd;
static int resblks_f(int argc, char **argv);

static int
resblks_f(
	int			argc,
	char			**argv)
{
	xfs_fsop_resblks_t	res;
	long long		blks;

	if (argc == 2) {
		blks = cvtnum(file->geom.blocksize, file->geom.sectsize, argv[1]);
		if (blks < 0) {
			printf(_("non-numeric argument -- %s\n"), argv[1]);
			return 0;
		}
		res.resblks = blks;
		if (xfsctl(file->name, file->fd, XFS_IOC_SET_RESBLKS, &res) < 0) {
			perror("XFS_IOC_SET_RESBLKS");
			return 0;
		}
	} else if (xfsctl(file->name, file->fd, XFS_IOC_GET_RESBLKS, &res) < 0) {
		perror("XFS_IOC_GET_RESBLKS");
		return 0;
	}
	printf(_("reserved blocks = %llu\n"),
			(unsigned long long) res.resblks);
	printf(_("available reserved blocks = %llu\n"),
			(unsigned long long) res.resblks_avail);
	return 0;
}

void
resblks_init(void)
{
	resblks_cmd.name = "resblks";
	resblks_cmd.cfunc = resblks_f;
	resblks_cmd.argmin = 0;
	resblks_cmd.argmax = 1;
	resblks_cmd.flags = CMD_NOMAP_OK;
	resblks_cmd.args = _("[blocks]");
	resblks_cmd.oneline =
		_("get and/or set count of reserved filesystem blocks");

	if (expert)
		add_command(&resblks_cmd);
}
