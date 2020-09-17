// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2000-2001,2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "libxfs.h"
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
