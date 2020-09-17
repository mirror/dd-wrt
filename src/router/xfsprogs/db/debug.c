// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
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
