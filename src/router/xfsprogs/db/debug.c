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
#include "debug.h"
#include "output.h"

static int	debug_f(int argc, char **argv);

static const cmdinfo_t	debug_cmd =
	{ "debug", NULL, debug_f, 0, 1, 0, N_("[flagbits]"),
	  N_("set debug option bits"), NULL };

long	debug_state;

static int
debug_f(
	int	argc,
	char	**argv)
{
	char	*p;

	if (argc > 1) {
		debug_state = strtol(argv[1], &p, 0);
		if (*p != '\0') {
			dbprintf(_("bad value for debug %s\n"), argv[1]);
			return 0;
		}
	}
	dbprintf("debug = %ld\n", debug_state);
	return 0;
}

void
debug_init(void)
{
	add_command(&debug_cmd);
}
