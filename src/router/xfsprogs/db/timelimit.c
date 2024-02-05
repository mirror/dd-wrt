// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "libxfs.h"
#include "command.h"
#include "output.h"
#include "init.h"

enum show_what {
	SHOW_AUTO,
	SHOW_CLASSIC,
	SHOW_BIGTIME,
};


enum print_how {
	PRINT_RAW,
	PRINT_PRETTY,
	PRINT_COMPACT,
};

static void
show_limit(
	const char	*tag,
	int64_t		limit,
	enum print_how	how)
{
	if (how == PRINT_COMPACT) {
		dbprintf("%" PRId64 " ", limit);
		return;
	}

	if (how == PRINT_PRETTY && limit <= LONG_MAX && limit >= LONG_MIN) {
		time_t	tt = limit;
		char	*c;

		c = ctime(&tt);
		if (c) {
			dbprintf("%s = %24.24s\n", tag, c);
			return;
		}
	}

	dbprintf("%s = %" PRId64 "\n", tag, limit);
}

static void
show_limits(
	enum show_what	whatkind,
	enum print_how	how)
{
	enum print_how	grace_how = how;

	switch (whatkind) {
	case SHOW_AUTO:
		/* should never get here */
		break;
	case SHOW_CLASSIC:
		show_limit("time.min", XFS_LEGACY_TIME_MIN, how);
		show_limit("time.max", XFS_LEGACY_TIME_MAX, how);
		show_limit("dqtimer.min", XFS_DQ_LEGACY_EXPIRY_MIN, how);
		show_limit("dqtimer.max", XFS_DQ_LEGACY_EXPIRY_MAX, how);
		break;
	case SHOW_BIGTIME:
		show_limit("time.min",
				xfs_bigtime_to_unix(XFS_BIGTIME_TIME_MIN), how);
		show_limit("time.max",
				xfs_bigtime_to_unix(XFS_BIGTIME_TIME_MAX), how);
		show_limit("dqtimer.min",
				xfs_dq_bigtime_to_unix(XFS_DQ_BIGTIME_EXPIRY_MIN),
				how);
		show_limit("dqtimer.max",
				xfs_dq_bigtime_to_unix(XFS_DQ_BIGTIME_EXPIRY_MAX),
				how);
		break;
	}

	/* grace periods are always integers */
	if (grace_how != PRINT_COMPACT)
		grace_how = PRINT_RAW;
	show_limit("dqgrace.min", XFS_DQ_GRACE_MIN, grace_how);
	show_limit("dqgrace.min", XFS_DQ_GRACE_MAX, grace_how);

	if (how == PRINT_COMPACT)
		dbprintf("\n");
}

static int
timelimit_f(
	int		argc,
	char		**argv)
{
	enum show_what	whatkind = SHOW_AUTO;
	enum print_how	how = PRINT_RAW;
	int		i;

	for (i = 1; i < argc; i++) {
		if (!strcmp("--classic", argv[i]))
			whatkind = SHOW_CLASSIC;
		else if (!strcmp("--bigtime", argv[i]))
			whatkind = SHOW_BIGTIME;
		else if (!strcmp("--pretty", argv[i]))
			how = PRINT_PRETTY;
		else if (!strcmp("--compact", argv[i]))
			how = PRINT_COMPACT;
		else {
			dbprintf(_("%s: bad option for timelimit command\n"),
					argv[i]);
			return 1;
		}
	}

	if (whatkind == SHOW_AUTO) {
		if (xfs_has_bigtime(mp))
			whatkind = SHOW_BIGTIME;
		else
			whatkind = SHOW_CLASSIC;
	}

	show_limits(whatkind, how);
	return 0;
}

static void
timelimit_help(void)
{
	dbprintf(_(
"\n"
" Print the minimum and maximum supported values for inode timestamps,\n"
" disk quota expiration timers, and disk quota grace periods supported\n"
" by this filesystem.\n"
"\n"
" Options:\n"
"   --classic -- Force printing of the classic time limits.\n"
"   --bigtime -- Force printing of the bigtime limits.\n"
"   --pretty  -- Pretty-print the time limits.\n"
"   --compact -- Print the limits in a single line.\n"
"\n"
));

}

static const cmdinfo_t	timelimit_cmd = {
	.name		= "timelimit",
	.cfunc		= timelimit_f,
	.argmin		= 0,
	.argmax		= -1,
	.canpush	= 0,
	.args		= N_("[--classic|--bigtime] [--pretty]"),
	.oneline	= N_("display timestamp limits"),
	.help		= timelimit_help,
};

void
timelimit_init(void)
{
	add_command(&timelimit_cmd);
}
