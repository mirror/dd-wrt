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

#include "path.h"
#include "command.h"
#include "input.h"
#include "init.h"

char	*progname;
int	exitcode;
int	expert;
bool	foreign_allowed = false;

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
		_("Usage: %s [-V] [-x] [-f] [-p prog] [-c cmd]... [-d project]... [path]\n"),
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
		/* skip project quota entries */
		if ((fs_path->fs_flags & FS_PROJECT_PATH))
			continue;

		/* only consider foreign filesystems if told so */
		if (!foreign_allowed && (fs_path->fs_flags & FS_FOREIGN))
			continue;

		/* We can use this one */
		break;
	} while (index < fs_count);

	if (fs_path->fs_flags & FS_PROJECT_PATH)
		return 0;
	if (!foreign_allowed && (fs_path->fs_flags & FS_FOREIGN))
		return 0;
	if (index > fs_count)
		return 0;
	return index;
}

static int
init_check_command(
	const cmdinfo_t	*ct)
{
	if (!fs_path)
		return 1;

	/* Always run commands that are valid for all fs types. */
	if (ct->flags & CMD_ALL_FSTYPES)
		return 1;

	/* If it's an XFS filesystem, always run the command. */
	if (!(fs_path->fs_flags & FS_FOREIGN))
		return 1;

	/* If the user specified foreign filesystems are ok (-f), run cmd. */
	if (foreign_allowed &&
	    (ct->flags & CMD_FLAG_FOREIGN_OK))
		return 1;

	/* If cmd not allowed on foreign fs, regardless of -f flag, skip it. */
	if (!(ct->flags & CMD_FLAG_FOREIGN_OK)) {
		fprintf(stderr, _("%s: command is for XFS filesystems only\n"),
			ct->name);
		return 0;
	}

	/* foreign fs, but cmd only allowed via -f flag. Skip it. */
	fprintf(stderr,
		_("%s: foreign filesystem. Invoke xfs_quota with -f to enable.\n"),
		ct->name);
	return 0;
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

	while ((c = getopt(argc, argv, "c:d:D:fP:p:t:xV")) != EOF) {
		switch (c) {
		case 'c':	/* commands */
			add_user_command(optarg);
			break;
		case 'd':
			add_project_opt(optarg);
			break;
		case 'f':
			foreign_allowed = true;
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
	add_check_command(init_check_command);

	/*
	 * Ensure that global commands don't end up with an invalid path pointer
	 * by setting the default device at the first specified on the CLI
	 */
	if (argc != optind)
		fs_path = fs_table_lookup(argv[optind], FS_MOUNT_POINT);
	else
		fs_path = &fs_table[0];
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
