// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2012 Red Hat, Inc.
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t sync_range_cmd;

static void
sync_range_help(void)
{
	printf(_(
"\n"
" Trigger specific writeback commands on a range of the current file\n"
"\n"
" With no options, the SYNC_FILE_RANGE_WRITE is implied.\n"
" -a -- wait for IO to finish after writing (SYNC_FILE_RANGE_WAIT_AFTER).\n"
" -b -- wait for IO to finish before writing (SYNC_FILE_RANGE_WAIT_BEFORE).\n"
" -w -- write dirty data in range (SYNC_FILE_RANGE_WRITE).\n"
"\n"));
}

static int
sync_range_f(
	int		argc,
	char		**argv)
{
	off64_t		offset = 0, length = 0;
	int		c, sync_mode = 0;
	size_t		blocksize, sectsize;

	while ((c = getopt(argc, argv, "abw")) != EOF) {
		switch (c) {
		case 'a':
			sync_mode = SYNC_FILE_RANGE_WAIT_AFTER;
			break;
		case 'b':
			sync_mode = SYNC_FILE_RANGE_WAIT_BEFORE;
			break;
		case 'w':
			sync_mode = SYNC_FILE_RANGE_WRITE;
			break;
		default:
			exitcode = 1;
			return command_usage(&sync_range_cmd);
		}
	}

	/* default to just starting writeback on the range */
	if (!sync_mode)
		sync_mode = SYNC_FILE_RANGE_WRITE;

	if (optind != argc - 2) {
		exitcode = 1;
		return command_usage(&sync_range_cmd);
	}
	init_cvtnum(&blocksize, &sectsize);
	offset = cvtnum(blocksize, sectsize, argv[optind]);
	if (offset < 0) {
		printf(_("non-numeric offset argument -- %s\n"),
			argv[optind]);
		exitcode = 1;
		return 0;
	}
	optind++;
	length = cvtnum(blocksize, sectsize, argv[optind]);
	if (length < 0) {
		printf(_("non-numeric length argument -- %s\n"),
			argv[optind]);
		exitcode = 1;
		return 0;
	}

	if (sync_file_range(file->fd, offset, length, sync_mode) < 0) {
		perror("sync_file_range");
		exitcode = 1;
		return 0;
	}
	return 0;
}

void
sync_range_init(void)
{
	sync_range_cmd.name = "sync_range";
	sync_range_cmd.cfunc = sync_range_f;
	sync_range_cmd.argmin = 2;
	sync_range_cmd.argmax = -1;
	sync_range_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	sync_range_cmd.args = _("[-abw] off len");
	sync_range_cmd.oneline = _("Control writeback on a range of a file");
	sync_range_cmd.help = sync_range_help;

	add_command(&sync_range_cmd);
}
