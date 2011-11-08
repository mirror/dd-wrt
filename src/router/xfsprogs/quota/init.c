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

#include <xfs/path.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include "init.h"

char	*progname;
int	exitcode;
int	expert;

static char **projopts;	/* table of project names (cmdline) */
static int nprojopts;	/* number of entries in name table. */

static void
add_project_opt(
	char		*optarg)
{
	nprojopts++;
	projopts = realloc(projopts, sizeof(char*) * nprojopts);
	if (!projopts) {
		perror("realloc");
		exit(1);
	}
	projopts[nprojopts - 1] = optarg;
}

static void
usage(void)
{
	fprintf(stderr,
		_("Usage: %s [-p prog] [-c cmd]... [-d project]... [path]\n"),
		progname);
	exit(1);
}

void
init_cvtnum(
	unsigned int	*blocksize,
	unsigned int	*sectsize)
{
	*blocksize = 4096;
	*sectsize = 512;
}

static void
init_commands(void)
{
	edit_init();
	free_init();
	help_init();
	path_init();
	project_init();
	quot_init();
	quota_init();
	quit_init();
	report_init();
	state_init();
}

static int
init_args_command(
	int	index)
{
	if (index >= fs_count)
		return 0;

	do {
		fs_path = &fs_table[index++];
	} while ((fs_path->fs_flags & FS_PROJECT_PATH) && index < fs_count);

	if (fs_path->fs_flags & FS_PROJECT_PATH)
		return 0;
	if (index > fs_count)
		return 0;
	return index;
}

static void
init(
	int		argc,
	char		**argv)
{
	int		c;

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	while ((c = getopt(argc, argv, "c:d:D:P:p:t:xV")) != EOF) {
		switch (c) {
		case 'c':	/* commands */
			add_user_command(optarg);
			break;
		case 'd':
			add_project_opt(optarg);
			break;
		case 't':
			mtab_file = optarg;
			break;
		case 'D':
			projects_file = optarg;
			break;
		case 'P':
			projid_file = optarg;
			break;
		case 'p':
			progname = optarg;
			break;
		case 'x':
			expert++;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		default:
			usage();
		}
	}

	fs_table_initialise(argc - optind, &argv[optind], nprojopts, projopts);
	free(projopts);

	init_commands();
	add_args_command(init_args_command);
}

int
main(
	int		argc,
	char		**argv)
{
	init(argc, argv);
	command_loop();
	return exitcode;
}
