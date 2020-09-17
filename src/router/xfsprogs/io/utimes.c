// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Deepa Dinamani
 * All Rights Reserved.
 */

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"

static cmdinfo_t utimes_cmd;

static void
utimes_help(void)
{
	printf(_(
"\n"
" Update file atime and mtime of the current file with nansecond precision.\n"
"\n"
" Usage: utimes atime_sec atime_nsec mtime_sec mtime_nsec.\n"
" *_sec: Seconds elapsed since 1970-01-01 00:00:00 UTC.\n"
" *_nsec: Nanoseconds since the corresponding *_sec.\n"
"\n"));
}

static int
utimes_f(
	int		argc,
	char		**argv)
{
	struct timespec t[2];
	int result;

	/* Get the timestamps */
	result = timespec_from_string(argv[1], argv[2], &t[0]);
	if (result) {
		fprintf(stderr, "Bad value for atime\n");
		exitcode = 1;
		return 0;
	}
	result = timespec_from_string(argv[3], argv[4], &t[1]);
	if (result) {
		fprintf(stderr, "Bad value for mtime\n");
		exitcode = 1;
		return 0;
	}

	/* Call futimens to update time. */
	if (futimens(file->fd, t)) {
		perror("futimens");
		exitcode = 1;
		return 0;
	}

	return 0;
}

void
utimes_init(void)
{
	utimes_cmd.name = "utimes";
	utimes_cmd.cfunc = utimes_f;
	utimes_cmd.argmin = 4;
	utimes_cmd.argmax = 4;
	utimes_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	utimes_cmd.args = _("atime_sec atime_nsec mtime_sec mtime_nsec");
	utimes_cmd.oneline = _("Update file times of the current file");
	utimes_cmd.help = utimes_help;

	add_command(&utimes_cmd);
}
