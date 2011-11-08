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
#include "echo.h"
#include "output.h"

static int	echo_f(int argc, char **argv);

static const cmdinfo_t	echo_cmd =
	{ "echo", NULL, echo_f, 0, -1, 0, N_("[args]..."),
	  N_("echo arguments"), NULL };

/*ARGSUSED*/
static int
echo_f(
	int	argc,
	char	**argv)
{
	char	*c;

	for (c = *(++argv); c; c = *(++argv))
		dbprintf("%s ", c);
	dbprintf("\n");
	return 0;
}

void
echo_init(void)
{
	add_command(&echo_cmd);
}
