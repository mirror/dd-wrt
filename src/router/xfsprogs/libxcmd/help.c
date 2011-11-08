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

static cmdinfo_t help_cmd;
static void help_onecmd(const char *cmd, const cmdinfo_t *ct);
static void help_oneline(const char *cmd, const cmdinfo_t *ct);

static void
help_all(void)
{
	const cmdinfo_t	*ct;

	for (ct = cmdtab; ct < &cmdtab[ncmds]; ct++)
		help_oneline(ct->name, ct);
	printf(_("\nUse 'help commandname' for extended help.\n"));
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
		printf(_("command %s not found\n"), argv[1]);
		return 0;
	}
	help_onecmd(argv[1], ct);
	return 0;
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
		printf("%s ", cmd);
	else {
		printf("%s ", ct->name);
		if (ct->altname)
			printf("(or %s) ", ct->altname);
	}
	if (ct->args)
		printf("%s ", ct->args);
	printf("-- %s\n", ct->oneline);
}

void
help_init(void)
{
	help_cmd.name = "help";
	help_cmd.altname = "?";
	help_cmd.cfunc = help_f;
	help_cmd.argmin = 0;
	help_cmd.argmax = 1;
	help_cmd.flags = CMD_FLAG_GLOBAL;
	help_cmd.args = _("[command]");
	help_cmd.oneline = _("help for one or all commands");

	add_command(&help_cmd);
}
