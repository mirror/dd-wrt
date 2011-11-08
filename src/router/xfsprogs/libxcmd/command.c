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

cmdinfo_t	*cmdtab;
int		ncmds;

static argsfunc_t	args_func;
static checkfunc_t	check_func;
static int		ncmdline;
static char		**cmdline;

static int
compare(const void *a, const void *b)
{
	return strcmp(((const cmdinfo_t *)a)->name,
		      ((const cmdinfo_t *)b)->name);
}

void
add_command(
	const cmdinfo_t	*ci)
{
	cmdtab = realloc((void *)cmdtab, ++ncmds * sizeof(*cmdtab));
	cmdtab[ncmds - 1] = *ci;
	qsort(cmdtab, ncmds, sizeof(*cmdtab), compare);
}

static int
check_command(
	const cmdinfo_t	*ci)
{
	if (check_func)
		return check_func(ci);
	return 1;
}

void
add_check_command(
	checkfunc_t	cf)
{
	check_func = cf;
}

int
command_usage(
	const cmdinfo_t *ci)
{
	printf("%s %s -- %s\n", ci->name, ci->args, ci->oneline);
	return 0;
}

int
command(
	const cmdinfo_t	*ct,
	int		argc,
	char		**argv)
{
	char		*cmd = argv[0];

	if (!check_command(ct))
		return 0;

	if (argc-1 < ct->argmin || (ct->argmax != -1 && argc-1 > ct->argmax)) {
		if (ct->argmax == -1)
			fprintf(stderr,
	_("bad argument count %d to %s, expected at least %d arguments\n"),
				argc-1, cmd, ct->argmin);
		else if (ct->argmin == ct->argmax)
			fprintf(stderr,
	_("bad argument count %d to %s, expected %d arguments\n"),
				argc-1, cmd, ct->argmin);
		else
			fprintf(stderr,
	_("bad argument count %d to %s, expected between %d and %d arguments\n"),
			argc-1, cmd, ct->argmin, ct->argmax);
		return 0;
	}
	platform_getoptreset();
	return ct->cfunc(argc, argv);
}

const cmdinfo_t *
find_command(
	const char	*cmd)
{
	cmdinfo_t	*ct;

	for (ct = cmdtab; ct < &cmdtab[ncmds]; ct++) {
		if (strcmp(ct->name, cmd) == 0 ||
		    (ct->altname && strcmp(ct->altname, cmd) == 0))
			return (const cmdinfo_t *)ct;
	}
	return NULL;
}

void
add_user_command(char *optarg)
{
	ncmdline++;
	cmdline = realloc(cmdline, sizeof(char*) * (ncmdline));
	if (!cmdline) {
		perror("realloc");
		exit(1);
	}
	cmdline[ncmdline-1] = optarg;
}

static int
args_command(
	int	index)
{
	if (args_func)
		return args_func(index);
	return 0;
}

void
add_args_command(
	argsfunc_t	af)
{
	args_func = af;
}

void
command_loop(void)
{
	int		c, i, j = 0, done = 0;
	char		*input;
	char		**v;
	const cmdinfo_t	*ct;

	for (i = 0; !done && i < ncmdline; i++) {
		input = strdup(cmdline[i]);
		if (!input) {
			fprintf(stderr,
				_("cannot strdup command '%s': %s\n"),
				cmdline[i], strerror(errno));
			exit(1);
		}
		v = breakline(input, &c);
		if (c) {
			ct = find_command(v[0]);
			if (ct) {
				if (ct->flags & CMD_FLAG_GLOBAL)
					done = command(ct, c, v);
				else {
					j = 0;
					while (!done && (j = args_command(j)))
						done = command(ct, c, v);
				}
			} else
				fprintf(stderr, _("command \"%s\" not found\n"),
					v[0]);
		}
		doneline(input, v);
	}
	if (cmdline) {
		free(cmdline);
		return;
	}
	while (!done) {
		if ((input = fetchline()) == NULL)
			break;
		v = breakline(input, &c);
		if (c) {
			ct = find_command(v[0]);
			if (ct)
				done = command(ct, c, v);
			else
				fprintf(stderr, _("command \"%s\" not found\n"),
					v[0]);
		}
		doneline(input, v);
	}
}
