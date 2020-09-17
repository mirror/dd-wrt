// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include "platform_defs.h"
#include "command.h"
#include "../quota/init.h"

static cmdinfo_t quit_cmd;

/* ARGSUSED */
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
	quit_cmd.name = "quit";
	quit_cmd.altname = "q";
	quit_cmd.cfunc = quit_f;
	quit_cmd.argmin = -1;
	quit_cmd.argmax = -1;
	quit_cmd.flags = CMD_FLAG_ONESHOT | CMD_FLAG_LIBRARY;
	quit_cmd.oneline = _("exit the program");

	add_command(&quit_cmd);
}
