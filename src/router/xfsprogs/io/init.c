// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include <pthread.h>
#include "platform_defs.h"
#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

char	*progname;
int	exitcode;
int	expert;
static int	idlethread;
size_t	pagesize;
struct timeval stopwatch;

static void
usage(void)
{
	fprintf(stderr,
_("Usage: %s [-adfinrRstVx] [-m mode] [-p prog] [[-c|-C] cmd]... file\n"),
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
	bulkstat_init();
	copy_range_init();
	cowextsize_init();
	encrypt_init();
	fadvise_init();
	fiemap_init();
	file_init();
	flink_init();
	freeze_init();
	fsmap_init();
	fsuuid_init();
	fsync_init();
	getrusage_init();
	help_init();
	imap_init();
	inject_init();
	label_init();
	log_writes_init();
	madvise_init();
	mincore_init();
	mmap_init();
	open_init();
	parent_init();
	pread_init();
	prealloc_init();
	pwrite_init();
	quit_init();
	readdir_init();
	reflink_init();
	repair_init();
	resblks_init();
	scrub_init();
	seek_init();
	sendfile_init();
	shutdown_init();
	stat_init();
	swapext_init();
	sync_init();
	sync_range_init();
	truncate_init();
	utimes_init();
	crc32cselftest_init();
}

/*
 * This allows xfs_io commands specified on the command line to be run on every
 * open file in the file table. Commands that should not be iterated across all
 * open files need to specify CMD_FLAG_ONESHOT in their command flags.
 */
static int
filetable_iterator(
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

static void
init(
	int		argc,
	char		**argv)
{
	int		c, flags = 0;
	char		*sp;
	mode_t		mode = 0600;
	struct xfs_fsop_geom geometry = { 0 };
	struct fs_path	fsp;

	progname = basename(argv[0]);
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	pagesize = getpagesize();
	gettimeofday(&stopwatch, NULL);

	fs_table_initialise(0, NULL, 0, NULL);
	while ((c = getopt(argc, argv, "ac:C:dFfiLm:p:PnrRstTVx")) != EOF) {
		switch (c) {
		case 'a':
			flags |= IO_APPEND;
			break;
		case 'c':
			add_user_command(optarg);
			break;
		case 'C':
			add_oneshot_user_command(optarg);
			break;
		case 'd':
			flags |= IO_DIRECT;
			break;
		case 'F':
			/* Ignored / deprecated now, handled automatically */
			break;
		case 'f':
			flags |= IO_CREAT;
			break;
		case 'i':
			idlethread = 1;
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
		case 'P':
			flags |= IO_PATH;
			break;
		case 'L':
			flags |= IO_NOFOLLOW;
			break;
		case 'R':
			flags |= IO_REALTIME;
			break;
		case 'T':
			flags |= IO_TMPFILE;
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
		c = openfile(argv[optind], &geometry, flags, mode, &fsp);
		if (c < 0)
			exit(1);
		if (!platform_test_xfs_fd(c))
			flags |= IO_FOREIGN;
		if (addfile(argv[optind], c, &geometry, flags, &fsp) < 0)
			exit(1);
		optind++;
	}

	init_commands();
	add_command_iterator(filetable_iterator);
	add_check_command(init_check_command);
}

/*
 * The purpose of this idle thread is to test io from a multi threaded process.
 * With single threaded process, the file table is not shared and file structs
 * are not reference counted. Spawning an idle thread can help detecting file
 * struct reference leaks.
 */
static void *
idle_loop(void *arg)
{
	for (;;)
		pause();
	return NULL;
}

static void
start_idle_thread(void)
{
	pthread_t t;

	if (pthread_create(&t, NULL, idle_loop, NULL)) {
		fprintf(stderr, "Error creating idle thread\n");
		exit(1);
	}
}

int
main(
	int	argc,
	char	**argv)
{
	init(argc, argv);
	if (idlethread)
		start_idle_thread();
	command_loop();
	return exitcode;
}
