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
#include "help.h"
#include "output.h"

static void	help_all(void);
static void	help_onecmd(const char *cmd, const cmdinfo_t *ct);
static int	help_f(int argc, char **argv);
static void	help_oneline(const char *cmd, const cmdinfo_t *ct);

static const cmdinfo_t	help_cmd =
	{ "help", "?", help_f, 0, 1, 0, N_("[command]"),
	  N_("help for one or all commands"), NULL };

static void
help_all(void)
{
	const cmdinfo_t	*ct;

	for (ct = cmdtab; ct < &cmdtab[ncmds]; ct++)
		help_oneline(ct->name, ct);
	dbprintf(_("\nUse 'help commandname' for extended help.\n"));
}

static int
help_f(
	int		argc,
	char		**argv)
{
	const cmdinfo_t	*ct;

	if (argc == 1) {
		help_all();
		return 0;
	}
	ct = find_command(argv[1]);
	if (ct == NULL) {
		dbprintf(_("command %s not found\n"), argv[1]);
		return 0;
	}
	help_onecmd(argv[1], ct);
	return 0;
}

void
help_init(void)
{
	add_command(&help_cmd);
}

static void
help_onecmd(
	const char	*cmd,
	const cmdinfo_t	*ct)
{
	help_oneline(cmd, ct);
	if (ct->help)
		ct->help();
}

static void
help_oneline(
	const char	*cmd,
	const cmdinfo_t	*ct)
{
	if (cmd)
		dbprintf("%s ", cmd);
	else {
		dbprintf("%s ", ct->name);
		if (ct->altname)
			dbprintf(_("(or %s) "), ct->altname);
	}
	if (ct->args)
		dbprintf("%s ", ct->args);
	dbprintf("-- %s\n", ct->oneline);
}
