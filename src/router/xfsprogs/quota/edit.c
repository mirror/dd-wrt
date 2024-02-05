// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */

#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include "input.h"
#include "command.h"
#include "init.h"
#include "quota.h"

static cmdinfo_t limit_cmd;
static cmdinfo_t restore_cmd;
static cmdinfo_t timer_cmd;
static cmdinfo_t warn_cmd;

static void
limit_help(void)
{
	printf(_(
"\n"
" modify quota limits for the specified user\n"
"\n"
" Example:\n"
" 'limit bsoft=100m bhard=110m tanya\n"
"\n"
" Changes the soft and/or hard block limits, inode limits and/or realtime\n"
" block limits that are currently being used for the specified user, group,\n"
" or project.  The filesystem identified by the current path is modified.\n"
" -d -- set the default values, used the first time a file is created\n"
" -g -- modify group quota limits\n"
" -p -- modify project quota limits\n"
" -u -- modify user quota limits\n"
" The block limit values can be specified with a units suffix - accepted\n"
" units are: k (kilobytes), m (megabytes), g (gigabytes), and t (terabytes).\n"
" The user/group/project can be specified either by name or by number.\n"
"\n"));
}

static void
timer_help(void)
{
	printf(_(
"\n"
" modify quota enforcement timeout for the current filesystem\n"
"\n"
" Example:\n"
" 'timer -i 3days'\n"
" (soft inode limit timer is changed to 3 days)\n"
"\n"
" Changes the timeout value associated with the block limits, inode limits\n"
" and/or realtime block limits for all users, groups, or projects on the\n"
" current filesystem.\n"
" As soon as a user consumes the amount of space or number of inodes set as\n"
" the soft limit, a timer is started.  If the timer expires and the user is\n"
" still over the soft limit, the soft limit is enforced as the hard limit.\n"
" The default timeout is 7 days.\n"
" -d -- set the default values, used the first time a file is created\n"
" -g -- modify group quota timer\n"
" -p -- modify project quota timer\n"
" -u -- modify user quota timer\n"
" -b -- modify the blocks-used timer\n"
" -i -- modify the inodes-used timer\n"
" -r -- modify the blocks-used timer for the (optional) realtime subvolume\n"
" The timeout value is specified as a number of seconds, by default.\n"
" However, a suffix may be used to alternatively specify minutes (m),\n"
" hours (h), days (d), or weeks (w) - either the full word or the first\n"
" letter of the word can be used.\n"
"\n"));
}

static void
warn_help(void)
{
	printf(_(
"\n"
" modify the number of quota warnings sent to the specified user\n"
"\n"
" Example:\n"
" 'warn 2 jimmy'\n"
" (tell the quota system that two warnings have been sent to user jimmy)\n"
"\n"
" Changes the warning count associated with the block limits, inode limits\n"
" and/or realtime block limits for the specified user, group, or project.\n"
" When a user has been warned the maximum number of times allowed, the soft\n"
" limit is enforced as the hard limit.  It is intended as an alternative to\n"
" the timeout system, where the system administrator updates a count of the\n"
" number of warnings issued to people, and they are penalised if the warnings\n"
" are ignored.\n"
" -d -- set maximum warning count, which triggers soft limit enforcement\n"
" -g -- set group quota warning count\n"
" -p -- set project quota warning count\n"
" -u -- set user quota warning count\n"
" -b -- set the blocks-used warning count\n"
" -i -- set the inodes-used warning count\n"
" -r -- set the blocks-used warn count for the (optional) realtime subvolume\n"
" The user/group/project can be specified either by name or by number.\n"
"\n"));
}

static uint32_t
id_from_string(
	char	*name,
	int	type)
{
	uint32_t	id = -1;
	const char	*type_name = "unknown type";

	switch (type) {
	case XFS_USER_QUOTA:
		type_name = "user";
		id = uid_from_string(name);
		break;
	case XFS_GROUP_QUOTA:
		type_name = "group";
		id = gid_from_string(name);
		break;
	case XFS_PROJ_QUOTA:
		type_name = "project";
		id = prid_from_string(name);
		break;
	default:
		ASSERT(0);
		break;
	}

	if (id == -1) {
		fprintf(stderr, _("%s: invalid %s name: %s\n"),
			type_name, progname, name);
		exitcode = 1;
	}
	return id;
}

static void
set_limits(
	uint32_t	id,
	uint		type,
	uint		mask,
	char		*dev,
	uint64_t	*bsoft,
	uint64_t	*bhard,
	uint64_t	*isoft,
	uint64_t	*ihard,
	uint64_t	*rtbsoft,
	uint64_t	*rtbhard)
{
	fs_disk_quota_t	d;

	memset(&d, 0, sizeof(d));
	d.d_version = FS_DQUOT_VERSION;
	d.d_id = id;
	d.d_flags = type;
	d.d_fieldmask = mask;
	d.d_blk_hardlimit = *bhard;
	d.d_blk_softlimit = *bsoft;
	d.d_ino_hardlimit = *ihard;
	d.d_ino_softlimit = *isoft;
	d.d_rtb_hardlimit = *rtbhard;
	d.d_rtb_softlimit = *rtbsoft;

	if (xfsquotactl(XFS_SETQLIM, dev, type, id, (void *)&d) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot set limits: %s\n"),
				progname, strerror(errno));
	}
}

/* extract number of blocks from an ascii string */
static int
extractb(
	char		*string,
	const char	*prefix,
	int		length,
	uint		blocksize,
	uint		sectorsize,
	uint64_t	*value)
{
	long long	v;
	char		*s = string;

	if (strncmp(string, prefix, length) == 0) {
		s = string + length + 1;
		v = cvtnum(blocksize, sectorsize, s);
		if (v == -1LL) {
			fprintf(stderr,
				_("%s: Error: could not parse size %s.\n"),
				progname, s);
			return 0;
		}
		*value = (uint64_t)v >> 9;	/* syscalls use basic blocks */
		if (v > 0 && *value == 0)
			fprintf(stderr, _("%s: Warning: `%s' in quota blocks is 0 (unlimited).\n"), progname, s);
		return 1;
	}
	return 0;
}

/* extract number of inodes from an ascii string */
static int
extracti(
	char		*string,
	const char	*prefix,
	int		length,
	uint64_t	*value)
{
	char		*sp, *s = string;

	if (strncmp(string, prefix, length) == 0) {
		s = string + length + 1;
		*value = strtoll(s, &sp, 0);
		return 1;
	}
	return 0;
}

static int
limit_f(
	int		argc,
	char		**argv)
{
	char		*name;
	uint32_t	id;
	uint64_t	bsoft, bhard, isoft, ihard, rtbsoft, rtbhard;
	int		c, type = 0, mask = 0, flags = 0;
	uint		bsize, ssize, endoptions;

	init_cvtnum(&bsize, &ssize);
	bsoft = bhard = isoft = ihard = rtbsoft = rtbhard = 0;
	while ((c = getopt(argc, argv, "dgpu")) != EOF) {
		switch (c) {
		case 'd':
			flags |= DEFAULTS_FLAG;
			break;
		case 'g':
			type |= XFS_GROUP_QUOTA;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			break;
		default:
			return command_usage(&limit_cmd);
		}
	}

	/*
	 * In the usual case, we need at least 2 more arguments -
	 * one (or more) limits and a user name/id.
	 * For setting defaults (-d) we don't want a user name/id.
	 */
	if (flags & DEFAULTS_FLAG) {
		if (argc < optind + 1)
			return command_usage(&limit_cmd);
		endoptions = 1;
	} else if (argc < optind + 2) {
		return command_usage(&limit_cmd);
	} else {
		endoptions = 2;
	}

	/*
	 * Extract limit values from remaining optional arguments.
	 */
	while (argc > optind + endoptions - 1) {
		char *s = argv[optind++];
		if (extractb(s, "bsoft=", 5, bsize, ssize, &bsoft))
			mask |= FS_DQ_BSOFT;
		else if (extractb(s, "bhard=", 5, bsize, ssize, &bhard))
			mask |= FS_DQ_BHARD;
		else if (extracti(s, "isoft=", 5, &isoft))
			mask |= FS_DQ_ISOFT;
		else if (extracti(s, "ihard=", 5, &ihard))
			mask |= FS_DQ_IHARD;
		else if (extractb(s, "rtbsoft=", 7, bsize, ssize, &rtbsoft))
			mask |= FS_DQ_RTBSOFT;
		else if (extractb(s, "rtbhard=", 7, bsize, ssize, &rtbhard))
			mask |= FS_DQ_RTBHARD;
		else {
			exitcode = 1;
			fprintf(stderr, _("%s: unrecognised argument %s\n"),
				progname, s);
			return 0;
		}
	}
	if (!mask) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot find any valid arguments\n"),
			progname);
		return 0;
	}

	name = (flags & DEFAULTS_FLAG) ? "0" : argv[optind++];

	if (!type) {
		type = XFS_USER_QUOTA;
	} else if (type != XFS_GROUP_QUOTA &&
	           type != XFS_PROJ_QUOTA &&
	           type != XFS_USER_QUOTA) {
		return command_usage(&limit_cmd);
	}


	id = id_from_string(name, type);
	if (id == -1)
		return 0;

	set_limits(id, type, mask, fs_path->fs_name,
		   &bsoft, &bhard, &isoft, &ihard, &rtbsoft, &rtbhard);
	return 0;
}

/*
 * Iterate through input file, restoring the limits.
 * File format is as follows:
 * fs = <device>
 * <numeric id> bsoft bhard isoft ihard [rtbsoft rtbhard]
 */
static void
restore_file(
	FILE		*fp,
	uint		type)
{
	char		buffer[512];
	char		dev[512];
	uint		mask;
	int		cnt;
	uint32_t	id;
	uint64_t	bsoft, bhard, isoft, ihard, rtbsoft, rtbhard;

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (strncmp("fs = ", buffer, 5) == 0) {
			/*
			 * Copy the device name to dev, strip off the trailing
			 * newline, and move on to the next line.
			 */
			strncpy(dev, buffer + 5, sizeof(dev) - 1);
			dev[strlen(dev) - 1] = '\0';
			continue;
		}
		rtbsoft = rtbhard = 0;
		cnt = sscanf(buffer, "%u %llu %llu %llu %llu %llu %llu\n",
				&id,
				(unsigned long long *)&bsoft,
				(unsigned long long *)&bhard,
				(unsigned long long *)&isoft,
				(unsigned long long *)&ihard,
				(unsigned long long *)&rtbsoft,
				(unsigned long long *)&rtbhard);
		if (cnt == 5 || cnt == 7) {
			mask = FS_DQ_ISOFT|FS_DQ_IHARD|FS_DQ_BSOFT|FS_DQ_BHARD;
			if (cnt == 7)
				mask |= FS_DQ_RTBSOFT|FS_DQ_RTBHARD;
			set_limits(id, type, mask, dev, &bsoft, &bhard,
					&isoft, &ihard, &rtbsoft, &rtbhard);
		}
	}
}

static int
restore_f(
	int		argc,
	char		**argv)
{
	FILE		*fp = stdin;
	char		*fname = NULL;
	int		c, type = 0;

	while ((c = getopt(argc, argv, "f:gpu")) != EOF) {
		switch (c) {
		case 'f':
			fname = optarg;
			break;
		case 'g':
			type |= XFS_GROUP_QUOTA;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			break;
		default:
			return command_usage(&restore_cmd);
		}
	}

	if (argc < optind)
		return command_usage(&restore_cmd);

	if (!type) {
		type = XFS_USER_QUOTA;
	} else if (type != XFS_GROUP_QUOTA &&
	           type != XFS_PROJ_QUOTA &&
	           type != XFS_USER_QUOTA) {
		return command_usage(&restore_cmd);
	}

	if (fname) {
		if ((fp = fopen(fname, "r")) == NULL) {
			exitcode = 1;
			fprintf(stderr, _("%s: fopen on %s failed: %s\n"),
				progname, fname, strerror(errno));
			return 0;
		}
	}

	restore_file(fp, type);

	if (fname)
		fclose(fp);
	return 0;
}

time64_t
decode_timer(
	const struct fs_disk_quota *d,
	__s32			timer_lo,
	__s8			timer_hi)
{
	if (d->d_fieldmask & FS_DQ_BIGTIME)
		return (uint32_t)timer_lo | (int64_t)timer_hi << 32;
	return timer_lo;
}

static inline void
encode_timer(
	const struct fs_disk_quota *d,
	__s32			*timer_lo,
	__s8			*timer_hi,
	time64_t		timer)
{
	*timer_lo = timer;
	if (d->d_fieldmask & FS_DQ_BIGTIME)
		*timer_hi = timer >> 32;
	else
		*timer_hi = 0;
}

static inline bool want_bigtime(time64_t timer)
{
	return timer > INT32_MAX || timer < INT32_MIN;
}

static void
encode_timers(
	struct fs_disk_quota	*d,
	time64_t		btimer,
	time64_t		itimer,
	time64_t		rtbtimer)
{
	d->d_fieldmask &= ~FS_DQ_BIGTIME;
	if (want_bigtime(btimer) || want_bigtime(itimer) ||
	    want_bigtime(rtbtimer))
		d->d_fieldmask |= FS_DQ_BIGTIME;

	encode_timer(d, &d->d_btimer, &d->d_btimer_hi, btimer);
	encode_timer(d, &d->d_itimer, &d->d_itimer_hi, itimer);
	encode_timer(d, &d->d_rtbtimer, &d->d_rtbtimer_hi, rtbtimer);
}

static void
set_timer(
	uint32_t		id,
	uint			type,
	uint			mask,
	char			*dev,
	time64_t		value)
{
	struct fs_disk_quota	d;
	time64_t		btimer, itimer, rtbtimer;

	memset(&d, 0, sizeof(d));

	/*
	 * If id is specified we are extending grace time by value
	 * Otherwise we are setting the default grace time
	 */
	if (id) {
		time_t	now;

		/* Get quota to find out whether user is past soft limits */
		if (xfsquotactl(XFS_GETQUOTA, dev, type, id, (void *)&d) < 0) {
			exitcode = 1;
			fprintf(stderr, _("%s: cannot get quota: %s\n"),
					progname, strerror(errno));
				return;
		}

		time(&now);

		btimer = decode_timer(&d, d.d_btimer, d.d_btimer_hi);
		itimer = decode_timer(&d, d.d_itimer, d.d_itimer_hi);
		rtbtimer = decode_timer(&d, d.d_rtbtimer, d.d_rtbtimer_hi);

		/* Only set grace time if user is already past soft limit */
		if (d.d_blk_softlimit && d.d_bcount > d.d_blk_softlimit)
			btimer = now + value;
		if (d.d_ino_softlimit && d.d_icount > d.d_ino_softlimit)
			itimer = now + value;
		if (d.d_rtb_softlimit && d.d_rtbcount > d.d_rtb_softlimit)
			rtbtimer = now + value;
	} else {
		btimer = value;
		itimer = value;
		rtbtimer = value;
	}

	d.d_version = FS_DQUOT_VERSION;
	d.d_flags = type;
	d.d_fieldmask = mask;
	d.d_id = id;
	encode_timers(&d, btimer, itimer, rtbtimer);

	if (xfsquotactl(XFS_SETQLIM, dev, type, id, (void *)&d) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot set timer: %s\n"),
				progname, strerror(errno));
	}
}

static int
timer_f(
	int		argc,
	char		**argv)
{
	time64_t	value;
	char		*name = NULL;
	uint32_t	id = 0;
	int		c, flags = 0, type = 0, mask = 0;

	while ((c = getopt(argc, argv, "bdgipru")) != EOF) {
		switch (c) {
		case 'd':
			flags |= DEFAULTS_FLAG;
			break;
		case 'b':
			mask |= FS_DQ_BTIMER;
			break;
		case 'i':
			mask |= FS_DQ_ITIMER;
			break;
		case 'r':
			mask |= FS_DQ_RTBTIMER;
			break;
		case 'g':
			type |= XFS_GROUP_QUOTA;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			break;
		default:
			return command_usage(&timer_cmd);
		}
	}

	 /*
	 * Older versions of the command did not accept -d|id|name,
	 * so in that case we assume we're setting default timer,
	 * and the last arg is the timer value.
	 *
	 * Otherwise, if the defaults flag is set, we expect 1 more arg for
	 * timer value ; if not, 2 more args: 1 for value, one for id/name.
	 */
	if (!(flags & DEFAULTS_FLAG) && (argc == optind + 1)) {
		value = cvttime(argv[optind++]);
	} else if (flags & DEFAULTS_FLAG) {
		if (argc != optind + 1)
			return command_usage(&timer_cmd);
		value = cvttime(argv[optind++]);
	} else if (argc == optind + 2) {
		value = cvttime(argv[optind++]);
		name = (flags & DEFAULTS_FLAG) ? "0" : argv[optind++];
	} else
		return command_usage(&timer_cmd);


	/* if none of -bir specified, set them all */
	if (!mask)
		mask = FS_DQ_TIMER_MASK;

	if (!type) {
		type = XFS_USER_QUOTA;
	} else if (type != XFS_GROUP_QUOTA &&
		   type != XFS_PROJ_QUOTA &&
		   type != XFS_USER_QUOTA) {
		return command_usage(&timer_cmd);
	}

	if (name)
		id = id_from_string(name, type);

	if (id == -1)
		return 0;

	set_timer(id, type, mask, fs_path->fs_name, value);
	return 0;
}

static void
set_warnings(
	uint32_t	id,
	uint		type,
	uint		mask,
	char		*dev,
	uint		value)
{
	fs_disk_quota_t	d;

	memset(&d, 0, sizeof(d));
	d.d_version = FS_DQUOT_VERSION;
	d.d_id = id;
	d.d_flags = type;
	d.d_fieldmask = mask;
	d.d_iwarns = value;
	d.d_bwarns = value;
	d.d_rtbwarns = value;

	if (xfsquotactl(XFS_SETQLIM, dev, type, id, (void *)&d) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot set warnings: %s\n"),
				progname, strerror(errno));
	}
}

static int
warn_f(
	int		argc,
	char		**argv)
{
	char		*name;
	uint32_t	id;
	uint		value;
	int		c, flags = 0, type = 0, mask = 0;

	while ((c = getopt(argc, argv, "bdgipru")) != EOF) {
		switch (c) {
		case 'd':
			flags |= DEFAULTS_FLAG;
			break;
		case 'b':
			mask |= FS_DQ_BWARNS;
			break;
		case 'i':
			mask |= FS_DQ_IWARNS;
			break;
		case 'r':
			mask |= FS_DQ_RTBWARNS;
			break;
		case 'g':
			type |= XFS_GROUP_QUOTA;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			break;
		default:
			return command_usage(&warn_cmd);
		}
	}

	/*
	 * In the usual case, we need at least 2 more arguments -
	 * one (or more) value and a user name/id.
	 * For setting defaults (-d) we don't want a user name/id.
	 */
	if (flags & DEFAULTS_FLAG) {
		if (argc != optind + 1)
			return command_usage(&warn_cmd);
	} else if (argc != optind + 2) {
		return command_usage(&warn_cmd);
	}

	value = atoi(argv[optind++]);
	name = (flags & DEFAULTS_FLAG) ? "0" : argv[optind++];

	if (!mask)
		mask = FS_DQ_WARNS_MASK;

	if (!type) {
		type = XFS_USER_QUOTA;
	} else if (type != XFS_GROUP_QUOTA &&
	           type != XFS_PROJ_QUOTA &&
	           type != XFS_USER_QUOTA) {
		return command_usage(&warn_cmd);
	}

	id = id_from_string(name, type);
	if (id == -1)
		return 0;

	set_warnings(id, type, mask, fs_path->fs_name, value);
	return 0;
}

void
edit_init(void)
{
	limit_cmd.name = "limit";
	limit_cmd.cfunc = limit_f;
	limit_cmd.argmin = 2;
	limit_cmd.argmax = -1;
	limit_cmd.args = \
	_("[-g|-p|-u] bsoft|bhard|isoft|ihard|rtbsoft|rtbhard=N -d|id|name");
	limit_cmd.oneline = _("modify quota limits");
	limit_cmd.help = limit_help;
	limit_cmd.flags = CMD_FLAG_FOREIGN_OK;

	restore_cmd.name = "restore";
	restore_cmd.cfunc = restore_f;
	restore_cmd.argmin = 0;
	restore_cmd.argmax = -1;
	restore_cmd.args = _("[-g|-p|-u] [-f file]");
	restore_cmd.oneline = _("restore quota limits from a backup file");
	restore_cmd.flags = CMD_FLAG_FOREIGN_OK;

	timer_cmd.name = "timer";
	timer_cmd.cfunc = timer_f;
	timer_cmd.argmin = 1;
	timer_cmd.argmax = -1;
	timer_cmd.args = _("[-bir] [-g|-p|-u] value [-d|id|name]");
	timer_cmd.oneline = _("set quota enforcement timeouts");
	timer_cmd.help = timer_help;
	timer_cmd.flags = CMD_FLAG_FOREIGN_OK;

	warn_cmd.name = "warn";
	warn_cmd.cfunc = warn_f;
	warn_cmd.argmin = 2;
	warn_cmd.argmax = -1;
	warn_cmd.args = _("[-bir] [-g|-p|-u] value -d|id|name");
	warn_cmd.oneline = _("get/set enforcement warning counter");
	warn_cmd.help = warn_help;

	if (expert) {
		add_command(&limit_cmd);
		add_command(&restore_cmd);
		add_command(&timer_cmd);
		add_command(&warn_cmd);
	}
}
