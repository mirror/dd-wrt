/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
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

#include <pwd.h>
#include <grp.h>
#include <ctype.h>
#include <xfs/input.h>
#include <xfs/command.h>
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

static void
set_limits(
	__uint32_t	id,
	uint		type,
	uint		mask,
	char		*dev,
	__uint64_t	*bsoft,
	__uint64_t	*bhard,
	__uint64_t	*isoft,
	__uint64_t	*ihard, 
	__uint64_t	*rtbsoft,
	__uint64_t	*rtbhard)
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

static void
set_user_limits(
	char		*name,
	uint		type,
	uint		mask,
	__uint64_t	*bsoft,
	__uint64_t	*bhard,
	__uint64_t	*isoft,
	__uint64_t	*ihard, 
	__uint64_t	*rtbsoft,
	__uint64_t	*rtbhard)
{
	uid_t		uid = uid_from_string(name);

	if (uid == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: invalid user name: %s\n"),
				progname, name);
	} else
		set_limits(uid, type, mask, fs_path->fs_name,
				bsoft, bhard, isoft, ihard, rtbsoft, rtbhard);
}

static void
set_group_limits(
	char		*name,
	uint		type,
	uint		mask,
	__uint64_t	*bsoft,
	__uint64_t	*bhard,
	__uint64_t	*isoft,
	__uint64_t	*ihard, 
	__uint64_t	*rtbsoft,
	__uint64_t	*rtbhard)
{
	gid_t		gid = gid_from_string(name);

	if (gid == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: invalid group name: %s\n"),
				progname, name);
	} else
		set_limits(gid, type, mask, fs_path->fs_name,
				bsoft, bhard, isoft, ihard, rtbsoft, rtbhard);
}

static void
set_project_limits(
	char		*name,
	uint		type,
	uint		mask,
	__uint64_t	*bsoft,
	__uint64_t	*bhard,
	__uint64_t	*isoft,
	__uint64_t	*ihard, 
	__uint64_t	*rtbsoft,
	__uint64_t	*rtbhard)
{
	prid_t		prid = prid_from_string(name);

	if (prid == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: invalid project name: %s\n"),
				progname, name);
	} else
		set_limits(prid, type, mask, fs_path->fs_name,
				bsoft, bhard, isoft, ihard, rtbsoft, rtbhard);
}

/* extract number of blocks from an ascii string */
static int
extractb(
	char		*string,
	const char	*prefix,
	int		length,
	uint		blocksize,
	uint		sectorsize,
	__uint64_t	*value)
{
	__uint64_t	v;
	char		*s = string;

	if (strncmp(string, prefix, length) == 0) {
		s = string + length + 1;
		v = (__uint64_t)cvtnum(blocksize, sectorsize, s);
		*value = v >> 9;	/* syscalls use basic blocks */
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
	__uint64_t	*value)
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
	__uint64_t	bsoft, bhard, isoft, ihard, rtbsoft, rtbhard;
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
			type = XFS_GROUP_QUOTA;
			break;
		case 'p':
			type = XFS_PROJ_QUOTA;
			break;
		case 'u':
			type = XFS_USER_QUOTA;
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

	if (!type)
		type = XFS_USER_QUOTA;

	switch (type) {
	case XFS_USER_QUOTA:
		set_user_limits(name, type, mask,
			&bsoft, &bhard, &isoft, &ihard, &rtbsoft, &rtbhard);
		break;
	case XFS_GROUP_QUOTA:
		set_group_limits(name, type, mask,
			&bsoft, &bhard, &isoft, &ihard, &rtbsoft, &rtbhard);
		break;
	case XFS_PROJ_QUOTA:
		set_project_limits(name, type, mask,
			&bsoft, &bhard, &isoft, &ihard, &rtbsoft, &rtbhard);
		break;
	}
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
	char		devbuffer[512];
	char		*dev = NULL;
	uint		mask;
	int		cnt;
	__uint32_t	id;
	__uint64_t	bsoft, bhard, isoft, ihard, rtbsoft, rtbhard;

	while (fgets(buffer, sizeof(buffer), fp) != NULL) {
		if (strncmp("fs = ", buffer, 5) == 0) {
			dev = strncpy(devbuffer, buffer+5, sizeof(devbuffer));
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
			type = XFS_GROUP_QUOTA;
			break;
		case 'p':
			type = XFS_PROJ_QUOTA;
			break;
		case 'u':
			type = XFS_USER_QUOTA;
			break;
		default:
			return command_usage(&restore_cmd);
		}
	}

	if (argc < optind)
		return command_usage(&restore_cmd);

	if (!type)
		type = XFS_USER_QUOTA;

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

static void
set_timer(
	uint		type,
	uint		mask,
	char		*dev,
	uint		value)
{
	fs_disk_quota_t	d;

	memset(&d, 0, sizeof(d));
	d.d_version = FS_DQUOT_VERSION;
	d.d_flags = type;
	d.d_fieldmask = mask;
	d.d_itimer = value;
	d.d_btimer = value;
	d.d_rtbtimer = value;

	if (xfsquotactl(XFS_SETQLIM, dev, type, 0, (void *)&d) < 0) {
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
	uint		value;
	int		c, type = 0, mask = 0;

	while ((c = getopt(argc, argv, "bgipru")) != EOF) {
		switch (c) {
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
			type = XFS_GROUP_QUOTA;
			break;
		case 'p':
			type = XFS_PROJ_QUOTA;
			break;
		case 'u':
			type = XFS_USER_QUOTA;
			break;
		default:
			return command_usage(&timer_cmd);
		}
	}

	if (argc != optind + 1)
		return command_usage(&timer_cmd);

	value = cvttime(argv[optind++]);

	if (!mask)
		mask = FS_DQ_TIMER_MASK;

	if (!type)
		type = XFS_USER_QUOTA;

	set_timer(type, mask, fs_path->fs_name, value);
	return 0;
}

static void
set_warnings(
	__uint32_t	id,
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

static void
set_user_warnings(
	char		*name,
	uint		type,
	uint		mask,
	uint		value)
{
	uid_t		uid = uid_from_string(name);

	if (uid == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: invalid user name: %s\n"),
				progname, name);
	} else
		set_warnings(uid, type, mask, fs_path->fs_name, value);
}

static void
set_group_warnings(
	char		*name,
	uint		type,
	uint		mask,
	uint		value)
{
	gid_t		gid = gid_from_string(name);

	if (gid == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: invalid group name: %s\n"),
				progname, name);
	} else
		set_warnings(gid, type, mask, fs_path->fs_name, value);
}

static void
set_project_warnings(
	char		*name,
	uint		type,
	uint		mask,
	uint		value)
{
	prid_t		prid = prid_from_string(name);

	if (prid == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: invalid project name: %s\n"),
				progname, name);
	} else
		set_warnings(prid, type, mask, fs_path->fs_name, value);
}

static int
warn_f(
	int		argc,
	char		**argv)
{
	char		*name;
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
			type = XFS_GROUP_QUOTA;
			break;
		case 'p':
			type = XFS_PROJ_QUOTA;
			break;
		case 'u':
			type = XFS_USER_QUOTA;
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

	if (!type)
		type = XFS_USER_QUOTA;

	switch (type) {
	case XFS_USER_QUOTA:
		set_user_warnings(name, type, mask, value);
		break;
	case XFS_GROUP_QUOTA:
		set_group_warnings(name, type, mask, value);
		break;
	case XFS_PROJ_QUOTA:
		set_project_warnings(name, type, mask, value);
		break;
	}
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
	_("[-gpu] bsoft|bhard|isoft|ihard|rtbsoft|rtbhard=N -d|id|name");
	limit_cmd.oneline = _("modify quota limits");
	limit_cmd.help = limit_help;

	restore_cmd.name = "restore";
	restore_cmd.cfunc = restore_f;
	restore_cmd.argmin = 0;
	restore_cmd.argmax = -1;
	restore_cmd.args = _("[-gpu] [-f file]");
	restore_cmd.oneline = _("restore quota limits from a backup file");

	timer_cmd.name = "timer";
	timer_cmd.cfunc = timer_f;
	timer_cmd.argmin = 2;
	timer_cmd.argmax = -1;
	timer_cmd.args = _("[-bir] [-gpu] value -d|id|name");
	timer_cmd.oneline = _("get/set quota enforcement timeouts");
	timer_cmd.help = timer_help;

	warn_cmd.name = "warn";
	warn_cmd.cfunc = warn_f;
	warn_cmd.argmin = 2;
	warn_cmd.argmax = -1;
	warn_cmd.args = _("[-bir] [-gpu] value -d|id|name");
	warn_cmd.oneline = _("get/set enforcement warning counter");
	warn_cmd.help = warn_help;

	if (expert) {
		add_command(&limit_cmd);
		add_command(&restore_cmd);
		add_command(&timer_cmd);
		add_command(&warn_cmd);
	}
}
