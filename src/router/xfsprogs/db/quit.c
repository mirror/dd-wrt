/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
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

#include <xfs/libxfs.h>
#include "command.h"
#include "quit.h"

static int	quit_f(int argc, char **argv);

static const cmdinfo_t	quit_cmd =
	{ "quit", "q", quit_f, 0, 0, 0, NULL,
	  N_("exit xfs_db"), NULL };

static int
quit_f(
	int	argc,
	char	**argv)
{
	return 1;
}

void
quit_init(void)
{
	add_command(&quit_cmd);
}
