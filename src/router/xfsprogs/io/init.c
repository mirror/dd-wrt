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
#include "init.h"
#include "io.h"

char	*progname;
int	exitcode;
int	expert;
size_t	pagesize;
struct timeval stopwatch;

void
usage(void)
{
	fprintf(stderr,
		_("Usage: %s [-adFfmrRstx] [-p prog] [-c cmd]... file\n"),
		progname);
	exit(1);
}

void
init_cvtnum(
	size_t *blocksize,
	size_t *sectsize)
{
	if (!file || (file->flags & IO_FOREIGN)) {
		*blocksize = 4096;
		*sectsize = 512;
	} else {
		*blocksize = file->geom.blocksize;
		*sectsize = file->geom.sectsize;
	}
}

static void
init_commands(void)
{
	attr_init();
	bmap_init();
	fadvise_init();
	file_init();
	freeze_init();
	fsync_init();
	getrusage_init();
	help_init();
	imap_init();
	inject_init();
	madvise_init();
	mincore_init();
	mmap_init();
	open_init();
	parent_init();
	pread_init();
	prealloc_init();
	fiemap_init();
	pwrite_init();
	quit_init();
	resblks_init();
	sendfile_init();
	shutdown_init();
	truncate_init();
}

static int
init_args_command(
	int	index)
{
	if (index >= filecount)
		return 0;
	file = &filetable[index++];
	return index;
}

static int
init_check_command(
	const cmdinfo_t	*ct)
{
	if (ct->flags & CMD_FLAG_GLOBAL)
		return 1;

	if (!file && !(ct->flags & CMD_NOFILE_OK)) {
		fprintf(stderr, _("no files are open, try 'help open'\n"));
		return 0;
	}
	if (!mapping && !(ct->flags & CMD_NOMAP_OK)) {
		fprintf(stderr, _("no mapped regions, try 'help mmap'\n"));
		return 0;
	}
	if (file && !(ct->flags & CMD_FOREIGN_OK) &&
					(file->flags & IO_FOREIGN)) {
		fprintf(stderr,
	_("foreign file active, %s command is for XFS filesystems only\n"),
			ct->name);
		return 0;
	}
	return 1;
}

void
init(
	int		argc,
	char		**argv)
{
	int		c, flags = 0;
	char		*sp;
	mode_t		mode = 0600;
	xfs_fsop_geom_t	geometry = { 0 };

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	pagesize = getpagesize();
	gettimeofday(&stopwatch, NULL);

	while ((c = getopt(argc, argv, "ac:dFfmp:nrRstVx")) != EOF) {
		switch (c) {
		case 'a':
			flags |= IO_APPEND;
			break;
		case 'c':
			add_user_command(optarg);
			break;
		case 'd':
			flags |= IO_DIRECT;
			break;
		case 'F':
			flags |= IO_FOREIGN;
			break;
		case 'f':
			flags |= IO_CREAT;
			break;
		case 'm':
			mode = strtoul(optarg, &sp, 0);
			if (!sp || sp == optarg) {
				fprintf(stderr, _("non-numeric mode -- %s\n"),
					optarg);
				exit(1);
			}
			break;
		case 'n':
			flags |= IO_NONBLOCK;
			break;
		case 'p':
			progname = optarg;
			break;
		case 'r':
			flags |= IO_READONLY;
			break;
		case 's':
			flags |= IO_OSYNC;
			break;
		case 't':
			flags |= IO_TRUNC;
			break;
		case 'R':
			flags |= IO_REALTIME;
			break;
		case 'x':
			expert = 1;
			break;
		case 'V':
			printf(_("%s version %s\n"), progname, VERSION);
			exit(0);
		default:
			usage();
		}
	}

	while (optind < argc) {
		if ((c = openfile(argv[optind], flags & IO_FOREIGN ?
					NULL : &geometry, flags, mode)) < 0)
			exit(1);
		if (addfile(argv[optind], c, &geometry, flags) < 0)
			exit(1);
		optind++;
	}

	init_commands();
	add_args_command(init_args_command);
	add_check_command(init_check_command);
}

int
main(
	int	argc,
	char	**argv)
{
	init(argc, argv);
	command_loop();
	return exitcode;
}
