// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 */
#include <stdbool.h>
#include "command.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <utmp.h>
#include "init.h"
#include "quota.h"

static cmdinfo_t dump_cmd;
static cmdinfo_t report_cmd;

static void
dump_help(void)
{
	printf(_(
"\n"
" create a backup file which contains quota limits information\n"
" -g -- dump out group quota limits\n"
" -p -- dump out project quota limits\n"
" -u -- dump out user quota limits (default)\n"
" -L -- lower ID bound to dump\n"
" -U -- upper ID bound to dump\n"
" -f -- write the dump out to the specified file\n"
"\n"));
}

static void
report_help(void)
{
	printf(_(
"\n"
" report used space and inodes, and quota limits, for a filesystem\n"
" Example:\n"
" 'report -igh'\n"
" (reports inode usage for all groups, in an easy-to-read format)\n"
" This command is the equivalent of the traditional repquota command, which\n"
" prints a summary of the disk usage and quotas for the current filesystem,\n"
" or all filesystems.\n"
" -a -- report for all mounted filesystems with quota enabled\n"
" -h -- report in a human-readable format\n"
" -n -- skip identifier-to-name translations, just report IDs\n"
" -N -- suppress the header from the output\n"
" -t -- terse output format, hides rows which are all zero\n"
" -L -- lower ID bound to report on\n"
" -U -- upper ID bound to report on\n"
" -l -- look up names for IDs in lower-upper range\n"
" -g -- report group usage and quota information\n"
" -p -- report project usage and quota information\n"
" -u -- report user usage and quota information\n"
" -b -- report blocks-used information only\n"
" -i -- report inodes-used information only\n"
" -r -- report realtime-blocks-used information only\n"
"\n"));
}

static int
get_dquot(
	struct fs_disk_quota *d,
	uint		id,
	uint		type,
	char		*dev,
	int		flags)
{
	int		cmd;

	if (flags & GETNEXTQUOTA_FLAG)
		cmd = XFS_GETNEXTQUOTA;
	else
		cmd = XFS_GETQUOTA;

	/* Fall back silently if XFS_GETNEXTQUOTA fails, warn on XFS_GETQUOTA */
	if (xfsquotactl(cmd, dev, type, id, (void *)d) < 0) {
		if (errno != ENOENT && errno != ENOSYS && errno != ESRCH &&
		    cmd == XFS_GETQUOTA)
			perror("XFS_GETQUOTA");
		return 0;
	}

	/* Did kernelspace wrap? */
	if (d->d_id < id)
		return 0;

	return 1;
}

static int
dump_file(
	FILE		*fp,
	struct fs_disk_quota *d,
	char		*dev)
{
	if (!d->d_blk_softlimit && !d->d_blk_hardlimit &&
	    !d->d_ino_softlimit && !d->d_ino_hardlimit &&
	    !d->d_rtb_softlimit && !d->d_rtb_hardlimit)
		return 1;
	fprintf(fp, "fs = %s\n", dev);
	/* this branch is for backward compatibility reasons */
	if (d->d_rtb_softlimit || d->d_rtb_hardlimit)
		fprintf(fp, "%-10d %7llu %7llu %7llu %7llu %7llu %7llu\n",
			d->d_id,
			(unsigned long long)d->d_blk_softlimit,
			(unsigned long long)d->d_blk_hardlimit,
			(unsigned long long)d->d_ino_softlimit,
			(unsigned long long)d->d_ino_hardlimit,
			(unsigned long long)d->d_rtb_softlimit,
			(unsigned long long)d->d_rtb_hardlimit);
	else
		fprintf(fp, "%-10d %7llu %7llu %7llu %7llu\n",
			d->d_id,
			(unsigned long long)d->d_blk_softlimit,
			(unsigned long long)d->d_blk_hardlimit,
			(unsigned long long)d->d_ino_softlimit,
			(unsigned long long)d->d_ino_hardlimit);

	return 1;
}

static void
dump_limits_any_type(
	FILE		*fp,
	uint		type,
	char		*dir,
	uint		lower,
	uint		upper)
{
	fs_path_t	*mount;
	struct fs_disk_quota d;
	uint		id = lower, flags = 0;

	if ((mount = fs_table_lookup(dir, FS_MOUNT_POINT)) == NULL) {
		exitcode = 1;
		fprintf(stderr, "%s: cannot find mount point %s\n",
			progname, dir);
		return;
	}

	while (get_dquot(&d, id, type, mount->fs_name,
				flags | GETNEXTQUOTA_FLAG) &&
			!(upper && (d.d_id > upper))) {
		dump_file(fp, &d, mount->fs_name);
		id = d.d_id + 1;
		flags |= GETNEXTQUOTA_FLAG;
	}

	if (flags & GETNEXTQUOTA_FLAG)
		return;

	/* Otherwise fall back to iterating over each uid/gid/prjid */
	switch (type) {
	case XFS_GROUP_QUOTA: {
			struct group *g;
			setgrent();
			while ((g = getgrent()) != NULL) {
				if (get_dquot(&d, g->gr_gid, type,
							mount->fs_name, 0) &&
						!(lower && (d.d_id < lower)) &&
						!(upper && (d.d_id > upper)))
					dump_file(fp, &d, mount->fs_name);
			}
			endgrent();
			break;
		}
	case XFS_PROJ_QUOTA: {
			struct fs_project *p;
			setprent();
			while ((p = getprent()) != NULL) {
				if (get_dquot(&d, p->pr_prid, type,
							mount->fs_name, 0) &&
						!(lower && (d.d_id < lower)) &&
						!(upper && (d.d_id > upper)))
					dump_file(fp, &d, mount->fs_name);
			}
			endprent();
			break;
		}
	case XFS_USER_QUOTA: {
			struct passwd *u;
			setpwent();
			while ((u = getpwent()) != NULL) {
				if (get_dquot(&d, u->pw_uid, type,
							mount->fs_name, 0) &&
						!(lower && (d.d_id < lower)) &&
						!(upper && (d.d_id > upper)))
					dump_file(fp, &d, mount->fs_name);
			}
			endpwent();
			break;
		}
	}
}

static int
dump_f(
	int		argc,
	char		**argv)
{
	FILE		*fp;
	char		*fname = NULL;
	uint		lower = 0, upper = 0;
	int		c, type = 0;

	while ((c = getopt(argc, argv, "f:gpuL:U:")) != EOF) {
		switch(c) {
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
		case 'L':
			lower = (uint)atoi(optarg);
			break;
		case 'U':
			upper = (uint)atoi(optarg);
			break;
		default:
			return command_usage(&dump_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&dump_cmd);

	if (!type) {
		type = XFS_USER_QUOTA;
	} else if (type != XFS_GROUP_QUOTA &&
	           type != XFS_PROJ_QUOTA &&
	           type != XFS_USER_QUOTA) {
		return command_usage(&dump_cmd);
	}

	if ((fp = fopen_write_secure(fname)) == NULL)
		return 0;

	dump_limits_any_type(fp, type, fs_path->fs_dir, lower, upper);

	if (fname)
		fclose(fp);

	return 0;
}

static void
report_header(
	FILE		*fp,
	uint		form,
	uint		type,
	fs_path_t	*mount,
	int		flags)
{
	char		*typename = type_to_string(type);
	char		scratch[64];
	uint		i, count;

	if (flags & NO_HEADER_FLAG)
		return;

	/* line 1 */
	fprintf(fp, _("%s quota on %s (%s)\n"),
		typename, mount->fs_dir, mount->fs_name);

	/* line 2 */
	for (i = 0; i < 10; i++)
		fputc(' ', fp);
	if (form & XFS_BLOCK_QUOTA)
		fprintf(fp, (flags & HUMAN_FLAG) ?
			"%13c %s %13c" : "%20c %s %20c",
			' ', form_to_string(XFS_BLOCK_QUOTA), ' ');
	if (form & XFS_INODE_QUOTA)
		fprintf(fp, (flags & HUMAN_FLAG) ?
			"%13c %s %13c" : "%20c %s %20c",
			' ', form_to_string(XFS_INODE_QUOTA), ' ');
	if (form & XFS_RTBLOCK_QUOTA)
		fprintf(fp, (flags & HUMAN_FLAG) ?
			"%9c %s %9c" : "%15c %s %15c",
			' ', form_to_string(XFS_RTBLOCK_QUOTA), ' ');
	fputc('\n', fp);

	/* line 3 */
	snprintf(scratch, sizeof(scratch), "%s ID", typename);
	fprintf(fp, "%-10s ", scratch);
	if (form & XFS_BLOCK_QUOTA)
		fprintf(fp, (flags & HUMAN_FLAG) ?
			_("  Used   Soft   Hard Warn/Grace   ") :
		_("      Used       Soft       Hard    Warn/Grace     "));
	if (form & XFS_INODE_QUOTA)
		fprintf(fp, (flags & HUMAN_FLAG) ?
			_("  Used   Soft   Hard Warn/Grace  ") :
		_("      Used       Soft       Hard    Warn/ Grace     "));
	if (form & XFS_RTBLOCK_QUOTA)
		fprintf(fp, (flags & HUMAN_FLAG) ?
			_("  Used   Soft   Hard Warn/Grace   ") :
		_("      Used       Soft       Hard    Warn/Grace     "));
	fputc('\n', fp);

	/* line 4 */
	for (i = 0; i < 10; i++)
		fputc('-', fp);
	fputc(' ', fp);
	count = (flags & HUMAN_FLAG) ? 33 : 50;
	if (form & XFS_BLOCK_QUOTA) {
		for (i = 0; i < count; i++)
			fputc('-', fp);
		fputc(' ', fp);
	}
	if (form & XFS_INODE_QUOTA) {
		for (i = 0; i < count; i++)
			fputc('-', fp);
		fputc(' ', fp);
	}
	if (form & XFS_RTBLOCK_QUOTA) {
		for (i = 0; i < count; i++)
			fputc('-', fp);
		fputc(' ', fp);
	}
	fputc('\n', fp);
}

static int
report_mount(
	FILE		*fp,
	struct fs_disk_quota *d,
	char		*name,
	uint		form,
	uint		type,
	fs_path_t	*mount,
	uint		flags)
{
	time64_t	timer;
	char		c[8], h[8], s[8];
	uint		qflags;
	int		count;

	if (flags & TERSE_FLAG) {
		count = 0;
		if ((form & XFS_BLOCK_QUOTA) && d->d_bcount)
			count++;
		if ((form & XFS_INODE_QUOTA) && d->d_icount)
			count++;
		if ((form & XFS_RTBLOCK_QUOTA) && d->d_rtbcount)
			count++;
		if (!count)
			return 0;
	}

	if (!(flags & NO_HEADER_FLAG))
		report_header(fp, form, type, mount, flags);

	if (flags & NO_LOOKUP_FLAG) {
		fprintf(fp, "#%-10u", d->d_id);
	} else {
		if (name == NULL) {
			if (type == XFS_USER_QUOTA) {
				struct passwd	*u = getpwuid(d->d_id);
				if (u)
					name = u->pw_name;
			} else if (type == XFS_GROUP_QUOTA) {
				struct group	*g = getgrgid(d->d_id);
				if (g)
					name = g->gr_name;
			} else if (type == XFS_PROJ_QUOTA) {
				fs_project_t	*p = getprprid(d->d_id);
				if (p)
					name = p->pr_name;
			}
		}
		/* If no name is found, print the id #num instead of (null) */
		if (name != NULL)
			fprintf(fp, "%-10s", name);
		else
			fprintf(fp, "#%-9u", d->d_id);
	}

	if (form & XFS_BLOCK_QUOTA) {
		timer = decode_timer(d, d->d_btimer, d->d_btimer_hi);
		qflags = (flags & HUMAN_FLAG);
		if (d->d_blk_hardlimit && d->d_bcount > d->d_blk_hardlimit)
			qflags |= LIMIT_FLAG;
		if (d->d_blk_softlimit && d->d_bcount > d->d_blk_softlimit)
			qflags |= QUOTA_FLAG;
		if (flags & HUMAN_FLAG)
			fprintf(fp, " %6s %6s %6s  %02d %8s",
				bbs_to_string(d->d_bcount, c, sizeof(c)),
				bbs_to_string(d->d_blk_softlimit, s, sizeof(s)),
				bbs_to_string(d->d_blk_hardlimit, h, sizeof(h)),
				d->d_bwarns,
				time_to_string(timer, qflags));
		else
			fprintf(fp, " %10llu %10llu %10llu     %02d %9s",
				(unsigned long long)d->d_bcount >> 1,
				(unsigned long long)d->d_blk_softlimit >> 1,
				(unsigned long long)d->d_blk_hardlimit >> 1,
				d->d_bwarns,
				time_to_string(timer, qflags));
	}
	if (form & XFS_INODE_QUOTA) {
		timer = decode_timer(d, d->d_itimer, d->d_itimer_hi);
		qflags = (flags & HUMAN_FLAG);
		if (d->d_ino_hardlimit && d->d_icount > d->d_ino_hardlimit)
			qflags |= LIMIT_FLAG;
		if (d->d_ino_softlimit && d->d_icount > d->d_ino_softlimit)
			qflags |= QUOTA_FLAG;
		if (flags & HUMAN_FLAG)
			fprintf(fp, " %6s %6s %6s  %02d %8s",
				num_to_string(d->d_icount, c, sizeof(c)),
				num_to_string(d->d_ino_softlimit, s, sizeof(s)),
				num_to_string(d->d_ino_hardlimit, h, sizeof(h)),
				d->d_iwarns,
				time_to_string(timer, qflags));
		else
			fprintf(fp, " %10llu %10llu %10llu     %02d %9s",
				(unsigned long long)d->d_icount,
				(unsigned long long)d->d_ino_softlimit,
				(unsigned long long)d->d_ino_hardlimit,
				d->d_iwarns,
				time_to_string(timer, qflags));
	}
	if (form & XFS_RTBLOCK_QUOTA) {
		timer = decode_timer(d, d->d_rtbtimer, d->d_rtbtimer_hi);
		qflags = (flags & HUMAN_FLAG);
		if (d->d_rtb_hardlimit && d->d_rtbcount > d->d_rtb_hardlimit)
			qflags |= LIMIT_FLAG;
		if (d->d_rtb_softlimit && d->d_rtbcount > d->d_rtb_softlimit)
			qflags |= QUOTA_FLAG;
		if (flags & HUMAN_FLAG)
			fprintf(fp, " %6s %6s %6s  %02d %8s",
				bbs_to_string(d->d_rtbcount, c, sizeof(c)),
				bbs_to_string(d->d_rtb_softlimit, s, sizeof(s)),
				bbs_to_string(d->d_rtb_hardlimit, h, sizeof(h)),
				d->d_rtbwarns,
				time_to_string(timer, qflags));
		else
			fprintf(fp, " %10llu %10llu %10llu     %02d %9s",
				(unsigned long long)d->d_rtbcount >> 1,
				(unsigned long long)d->d_rtb_softlimit >> 1,
				(unsigned long long)d->d_rtb_hardlimit >> 1,
				d->d_rtbwarns,
				time_to_string(timer, qflags));
	}
	fputc('\n', fp);
	return 1;
}

static void
report_user_mount(
	FILE		*fp,
	uint		form,
	fs_path_t	*mount,
	uint		lower,
	uint		upper,
	uint		flags)
{
	struct passwd	*u;
	struct fs_disk_quota	d;
	uint		id = lower;

	while (get_dquot(&d, id, XFS_USER_QUOTA, mount->fs_name,
				flags | GETNEXTQUOTA_FLAG) &&
			!(upper && (d.d_id > upper))) {
		report_mount(fp, &d, NULL, form, XFS_USER_QUOTA, mount, flags);
		id = d.d_id + 1;
		flags |= GETNEXTQUOTA_FLAG;
		flags |= NO_HEADER_FLAG;
	}

	/* No GETNEXTQUOTA support, iterate over all from password file */
	if (!(flags & GETNEXTQUOTA_FLAG)) {
		setpwent();
		while ((u = getpwent()) != NULL) {
			if (get_dquot(&d, u->pw_uid, XFS_USER_QUOTA,
						mount->fs_name, flags) &&
					!(lower && (d.d_id < lower)) &&
					!(upper && (d.d_id > upper))) {
				report_mount(fp, &d, u->pw_name, form,
						XFS_USER_QUOTA, mount, flags);
				flags |= NO_HEADER_FLAG;
			}
		}
		endpwent();
	}

	if (flags & NO_HEADER_FLAG)
		fputc('\n', fp);
}

static void
report_group_mount(
	FILE		*fp,
	uint		form,
	fs_path_t	*mount,
	uint		lower,
	uint		upper,
	uint		flags)
{
	struct group	*g;
	struct fs_disk_quota	d;
	uint		id = lower;

	while (get_dquot(&d, id, XFS_GROUP_QUOTA, mount->fs_name,
				flags | GETNEXTQUOTA_FLAG) &&
			!(upper && (d.d_id > upper))) {
		report_mount(fp, &d, NULL, form, XFS_GROUP_QUOTA, mount, flags);
		id = d.d_id + 1;
		flags |= GETNEXTQUOTA_FLAG;
		flags |= NO_HEADER_FLAG;
	}

	/* No GETNEXTQUOTA support, iterate over all from password file */
	if (!(flags & GETNEXTQUOTA_FLAG)) {
		setgrent();
		while ((g = getgrent()) != NULL) {
			if (get_dquot(&d, g->gr_gid, XFS_GROUP_QUOTA,
						mount->fs_name, flags) &&
					!(lower && (d.d_id < lower)) &&
					!(upper && (d.d_id > upper))) {
				report_mount(fp, &d, g->gr_name, form,
						XFS_GROUP_QUOTA, mount, flags);
				flags |= NO_HEADER_FLAG;
			}
		}
	}
	if (flags & NO_HEADER_FLAG)
		fputc('\n', fp);
	endgrent();
}

static void
report_project_mount(
	FILE		*fp,
	uint		form,
	fs_path_t	*mount,
	uint		lower,
	uint		upper,
	uint		flags)
{
	fs_project_t	*p;
	struct fs_disk_quota	d;
	uint		id = lower;

	while (get_dquot(&d, id, XFS_PROJ_QUOTA, mount->fs_name,
				flags | GETNEXTQUOTA_FLAG) &&
			!(upper && (d.d_id > upper))) {
		report_mount(fp, &d, NULL, form, XFS_PROJ_QUOTA, mount, flags);
		id = d.d_id + 1;
		flags |= GETNEXTQUOTA_FLAG;
		flags |= NO_HEADER_FLAG;
	}

	/* No GETNEXTQUOTA support, iterate over all */
	if (!(flags & GETNEXTQUOTA_FLAG)) {
		if (!getprprid(0)) {
			/*
			 * Print default project quota, even if projid 0
			 * isn't defined
			 */
			if (get_dquot(&d, 0, XFS_PROJ_QUOTA, mount->fs_name,
						flags) &&
					!(lower && (d.d_id < lower)) &&
					!(upper && (d.d_id > upper))) {
				report_mount(fp, &d, NULL, form, XFS_PROJ_QUOTA,
						mount, flags);
				flags |= NO_HEADER_FLAG;
			}
		}

		setprent();
		while ((p = getprent()) != NULL) {
			if (get_dquot(&d, p->pr_prid, XFS_PROJ_QUOTA,
						mount->fs_name, flags) &&
					!(lower && (d.d_id < lower)) &&
					!(upper && (d.d_id > upper))) {
				report_mount(fp, &d, p->pr_name, form,
						XFS_PROJ_QUOTA, mount, flags);
				flags |= NO_HEADER_FLAG;
			}
		}
		endprent();
	}

	if (flags & NO_HEADER_FLAG)
		fputc('\n', fp);
}

static void
report_any_type(
	FILE		*fp,
	uint		form,
	uint		type,
	char		*dir,
	uint		lower,
	uint		upper,
	uint		flags)
{
	fs_cursor_t	cursor;
	fs_path_t	*mount;

	if (type & XFS_USER_QUOTA) {
		fs_cursor_initialise(dir, FS_MOUNT_POINT, &cursor);
		while ((mount = fs_cursor_next_entry(&cursor))) {
			if (!foreign_allowed && (mount->fs_flags & FS_FOREIGN))
				continue;
			if (xfsquotactl(XFS_QSYNC, mount->fs_name,
						XFS_USER_QUOTA, 0, NULL) < 0
					&& errno != ENOENT && errno != ENOSYS)
				perror("XFS_QSYNC user quota");
			report_user_mount(fp, form, mount,
						lower, upper, flags);
		}
	}
	if (type & XFS_GROUP_QUOTA) {
		fs_cursor_initialise(dir, FS_MOUNT_POINT, &cursor);
		while ((mount = fs_cursor_next_entry(&cursor))) {
			if (!foreign_allowed && (mount->fs_flags & FS_FOREIGN))
				continue;
			if (xfsquotactl(XFS_QSYNC, mount->fs_name,
						XFS_GROUP_QUOTA, 0, NULL) < 0
					&& errno != ENOENT && errno != ENOSYS)
				perror("XFS_QSYNC group quota");
			report_group_mount(fp, form, mount,
						lower, upper, flags);
		}
	}
	if (type & XFS_PROJ_QUOTA) {
		fs_cursor_initialise(dir, FS_MOUNT_POINT, &cursor);
		while ((mount = fs_cursor_next_entry(&cursor))) {
			if (!foreign_allowed && (mount->fs_flags & FS_FOREIGN))
				continue;
			if (xfsquotactl(XFS_QSYNC, mount->fs_name,
						XFS_PROJ_QUOTA, 0, NULL) < 0
					&& errno != ENOENT && errno != ENOSYS)
				perror("XFS_QSYNC proj quota");
			report_project_mount(fp, form, mount,
						lower, upper, flags);
		}
	}
}

static int
report_f(
	int		argc,
	char		**argv)
{
	FILE		*fp = NULL;
	char		*fname = NULL;
	uint		lower = 0, upper = 0;
	bool		lookup = false;
	int		c, flags = 0, type = 0, form = 0;

	while ((c = getopt(argc, argv, "abdf:ghilL:NnprtuU:")) != EOF) {
		switch (c) {
		case 'f':
			fname = optarg;
			break;
		case 'b':
			form |= XFS_BLOCK_QUOTA;
			break;
		case 'i':
			form |= XFS_INODE_QUOTA;
			break;
		case 'r':
			form |= XFS_RTBLOCK_QUOTA;
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
		case 'a':
			flags |= ALL_MOUNTS_FLAG;
			break;
		case 'h':
			flags |= HUMAN_FLAG;
			break;
		case 'n':
			flags |= NO_LOOKUP_FLAG;
			break;
		case 'N':
			flags |= NO_HEADER_FLAG;
			break;
		case 't':
			flags |= TERSE_FLAG;
			break;
		case 'L':
			lower = (uint)atoi(optarg);
			flags |= NO_LOOKUP_FLAG;
			break;
		case 'U':
			upper = (uint)atoi(optarg);
			flags |= NO_LOOKUP_FLAG;
			break;
		case 'l':
			lookup = true;
			break;
		default:
			return command_usage(&report_cmd);
		}
	}

	if (!form)
		form = XFS_BLOCK_QUOTA;

	if (!type)
		type = XFS_USER_QUOTA | XFS_GROUP_QUOTA | XFS_PROJ_QUOTA;

	if (lookup)
		flags &= ~NO_LOOKUP_FLAG;

	if ((fp = fopen_write_secure(fname)) == NULL)
		return 0;

	if (argc == optind) {
		if (flags & ALL_MOUNTS_FLAG)
			report_any_type(fp, form, type, NULL,
					lower, upper, flags);
		else if (fs_path && (fs_path->fs_flags & FS_MOUNT_POINT))
			report_any_type(fp, form, type, fs_path->fs_dir,
					lower, upper, flags);
	} else while (argc > optind) {
		report_any_type(fp, form, type, argv[optind++],
				lower, upper, flags);
	}

	if (fname)
		fclose(fp);
	return 0;
}

void
report_init(void)
{
	dump_cmd.name = "dump";
	dump_cmd.cfunc = dump_f;
	dump_cmd.argmin = 0;
	dump_cmd.argmax = -1;
	dump_cmd.args = _("[-g|-p|-u] [-LU] [-f file]");
	dump_cmd.oneline = _("dump quota information for backup utilities");
	dump_cmd.help = dump_help;
	dump_cmd.flags = CMD_FLAG_FOREIGN_OK;

	report_cmd.name = "report";
	report_cmd.altname = "repquota";
	report_cmd.cfunc = report_f;
	report_cmd.argmin = 0;
	report_cmd.argmax = -1;
	report_cmd.args = _("[-bir] [-gpu] [-ahntlLNU] [-f file]");
	report_cmd.oneline = _("report filesystem quota information");
	report_cmd.help = report_help;
	report_cmd.flags = CMD_FLAG_ONESHOT | CMD_FLAG_FOREIGN_OK;

	if (expert) {
		add_command(&dump_cmd);
		add_command(&report_cmd);
	}
}
