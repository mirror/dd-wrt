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

#include <xfs/command.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>
#include "init.h"
#include "quota.h"

static cmdinfo_t quota_cmd;

static void
quota_help(void)
{
	printf(_(
"\n"
" display usage and quota information\n"
"\n"
" -g -- display group quota information\n"
" -p -- display project quota information\n"
" -u -- display user quota information\n"
" -b -- display number of blocks used\n"
" -i -- display number of inodes used\n"
" -r -- display number of realtime blocks used\n"
" -h -- report in a human-readable format\n"
" -n -- skip identifier-to-name translations, just report IDs\n"
" -N -- suppress the initial header\n"
" -v -- increase verbosity in reporting (also dumps zero values)\n"
" -f -- send output to a file\n"
" The (optional) user/group/project can be specified either by name or by\n"
" number (i.e. uid/gid/projid).\n"
"\n"));
}

static int
quota_mount(
	FILE		*fp,
	__uint32_t	id,
	char		*name,
	uint		form,
	uint		type,
	fs_path_t	*mount,
	uint		flags)
{
	fs_disk_quota_t	d;
	char		*dev = mount->fs_name;
	char		c[8], h[8], s[8];
	uint		qflags;
	int		count;

	xfsquotactl(XFS_QSYNC, dev, type, 0, NULL);
	if (xfsquotactl(XFS_GETQUOTA, dev, type, id, (void *)&d) < 0)
		return 0;

	if (!(flags & VERBOSE_FLAG)) {
		count = 0;
		if ((form & XFS_BLOCK_QUOTA) && d.d_bcount)
			count++;
		if ((form & XFS_INODE_QUOTA) && d.d_icount)
			count++;
		if ((form & XFS_RTBLOCK_QUOTA) && d.d_rtbcount)
			count++;
		if (!count)
			return 0;
	}

	if (!(flags & NO_HEADER_FLAG)) {
		fprintf(fp,
			_("Disk quotas for %s %s (%u)\nFilesystem%s"),
			type_to_string(type), name, id,
			(flags & HUMAN_FLAG) ? "  " : "         ");
		if (form & XFS_BLOCK_QUOTA)
			fprintf(fp, (flags & HUMAN_FLAG) ?
			_(" Blocks  Quota  Limit Warn/Time    ") :
	_("     Blocks      Quota      Limit  Warn/Time      "));
		if (form & XFS_INODE_QUOTA)
			fprintf(fp, (flags & HUMAN_FLAG) ?
			_("  Files  Quota  Limit Warn/Time    ") :
	_("      Files      Quota      Limit  Warn/Time      "));
		if  (form & XFS_RTBLOCK_QUOTA)
			fprintf(fp, (flags & HUMAN_FLAG) ?
			_("Realtime Quota  Limit Warn/Time    ") :
	_("   Realtime      Quota      Limit  Warn/Time      "));
		fputs("Mounted on\n", fp);
	}

	if (flags & HUMAN_FLAG) {
		count = fprintf(fp, "%-12s", dev);
		if (count > 13)
			fprintf(fp, "\n%12s", " ");
	} else {
		count = fprintf(fp, "%-19s", dev);
		if (count > 20)
			fprintf(fp, "\n%19s", " ");
	}

	if (form & XFS_BLOCK_QUOTA) {
		qflags = (flags & HUMAN_FLAG);
		if (d.d_blk_hardlimit && d.d_bcount > d.d_blk_hardlimit)
			qflags |= LIMIT_FLAG;
		if (d.d_blk_softlimit && d.d_bcount > d.d_blk_softlimit)
			qflags |= QUOTA_FLAG;
		if (flags & HUMAN_FLAG)
			fprintf(fp, " %6s %6s %6s  %02d %8s ",
				bbs_to_string(d.d_bcount, c, sizeof(c)),
				bbs_to_string(d.d_blk_softlimit, s, sizeof(s)),
				bbs_to_string(d.d_blk_hardlimit, h, sizeof(h)),
				d.d_bwarns,
				time_to_string(d.d_btimer, qflags));
		else
			fprintf(fp, " %10llu %10llu %10llu   %02d %9s ",
				(unsigned long long)d.d_bcount >> 1,
				(unsigned long long)d.d_blk_softlimit >> 1,
				(unsigned long long)d.d_blk_hardlimit >> 1,
				d.d_bwarns,
				time_to_string(d.d_btimer, qflags));
	}
	if (form & XFS_INODE_QUOTA) {
		qflags = (flags & HUMAN_FLAG);
		if (d.d_ino_hardlimit && d.d_icount > d.d_ino_hardlimit)
			qflags |= LIMIT_FLAG;
		if (d.d_ino_softlimit && d.d_icount > d.d_ino_softlimit)
			qflags |= QUOTA_FLAG;
		if (flags & HUMAN_FLAG)
			fprintf(fp, " %6s %6s %6s  %02d %8s ",
				num_to_string(d.d_icount, c, sizeof(c)),
				num_to_string(d.d_ino_softlimit, s, sizeof(s)),
				num_to_string(d.d_ino_hardlimit, h, sizeof(h)),
				d.d_iwarns,
				time_to_string(d.d_itimer, qflags));
		else
			fprintf(fp, " %10llu %10llu %10llu   %02d %9s ",
				(unsigned long long)d.d_icount,
				(unsigned long long)d.d_ino_softlimit,
				(unsigned long long)d.d_ino_hardlimit,
				d.d_iwarns,
				time_to_string(d.d_itimer, qflags));
	}
	if (form & XFS_RTBLOCK_QUOTA) {
		qflags = (flags & HUMAN_FLAG);
		if (d.d_rtb_hardlimit && d.d_rtbcount > d.d_rtb_hardlimit)
			qflags |= LIMIT_FLAG;
		if (d.d_rtb_softlimit && d.d_rtbcount > d.d_rtb_softlimit)
			qflags |= QUOTA_FLAG;
		if (flags & HUMAN_FLAG)
			fprintf(fp, " %6s %6s %6s  %02d %8s ",
				bbs_to_string(d.d_rtbcount, c, sizeof(c)),
				bbs_to_string(d.d_rtb_softlimit, s, sizeof(s)),
				bbs_to_string(d.d_rtb_hardlimit, h, sizeof(h)),
				d.d_rtbwarns,
				time_to_string(d.d_rtbtimer, qflags));
		else
			fprintf(fp, " %10llu %10llu %10llu   %02d %9s ",
				(unsigned long long)d.d_rtbcount >> 1,
				(unsigned long long)d.d_rtb_softlimit >> 1,
				(unsigned long long)d.d_rtb_hardlimit >> 1,
				d.d_rtbwarns,
				time_to_string(d.d_rtbtimer, qflags));
	}
	fprintf(fp, "%s\n", mount->fs_dir);
	return 1;
}

static void
quota(
	FILE		*fp,
	__uint32_t	id,
	char		*name,
	uint		form,
	uint		type,
	uint		flags)
{
	fs_cursor_t	cursor;
	fs_path_t	*path;

	fs_cursor_initialise(NULL, FS_MOUNT_POINT, &cursor);
	while ((path = fs_cursor_next_entry(&cursor))) {
		if (quota_mount(fp, id, name, form, type, path, flags))
			flags |= NO_HEADER_FLAG;
	}
}

static char *
getusername(
	uid_t		uid,
	int		numeric)
{
	static char	buffer[32];

	if (!numeric) {
		struct passwd	*u = getpwuid(uid);
		if (u)
			return u->pw_name;
	}
	snprintf(buffer, sizeof(buffer), "#%u", uid);
	return &buffer[0];
}

static void
quota_user_type(
	FILE		*fp,
	char		*name,
	uint		form,
	uint		type,
	uint		flags)
{
	struct passwd	*u;
	uid_t		id;

	if (name) {
		if (isdigit(name[0])) {
			id = atoi(name);
			name = getusername(id, flags & NO_LOOKUP_FLAG);
		} else if ((u = getpwnam(name))) {
			id = u->pw_uid;
			name = u->pw_name;
		} else {
			exitcode = 1;
			fprintf(stderr, _("%s: cannot find user %s\n"),
				progname, name);
			return;
		}
	} else {
		id = getuid();
		name = getusername(id, flags & NO_LOOKUP_FLAG);
	}

	quota(fp, id, name, form, type, flags);
}

static char *
getgroupname(
	gid_t		gid,
	int		numeric)
{
	static char	buffer[32];

	if (!numeric) {
		struct group	*g = getgrgid(gid);
		if (g)
			return g->gr_name;
	}
	snprintf(buffer, sizeof(buffer), "#%u", gid);
	return &buffer[0];
}

static void
quota_group_type(
	FILE		*fp,
	char		*name,
	uint		form,
	uint		type,
	uint		flags)
{
	struct group	*g;
	gid_t		gid, *gids = NULL;
	int		i, ngroups, dofree = 0;

	if (name) {
		if (isdigit(name[0])) {
			gid = atoi(name);
			name = getgroupname(gid, flags & NO_LOOKUP_FLAG);
		} else {
			if ((g = getgrnam(name))) {
				gid = g->gr_gid;
				name = g->gr_name;
			} else {
				exitcode = 1;
				fprintf(stderr, _("%s: cannot find group %s\n"),
					progname, name);
				return;
			}
		}
		gids = &gid;
		ngroups = 1;
	} else if ( ((ngroups = sysconf(_SC_NGROUPS_MAX)) < 0) ||
		    ((gids = malloc(ngroups * sizeof(gid_t))) == NULL) ||
		    ((ngroups = getgroups(ngroups, gids)) < 0)) {
		dofree = (gids != NULL);
		gid = getgid();
		gids = &gid;
		ngroups = 1;
	} else {
		dofree = (gids != NULL);
	}

	for (i = 0; i < ngroups; i++, name = NULL) {
		if (!name)
			name = getgroupname(gids[i], flags & NO_LOOKUP_FLAG);
		quota(fp, gids[i], name, form, type, flags);
	}

	if (dofree)
		free(gids);
}

static char *
getprojectname(
	prid_t		prid,
	int		numeric)
{
	static char	buffer[32];

	if (!numeric) {
		fs_project_t	*p = getprprid(prid);
		if (p)
			return p->pr_name;
	}
	snprintf(buffer, sizeof(buffer), "#%u", (unsigned int)prid);
	return &buffer[0];
}

static void
quota_proj_type(
	FILE		*fp,
	char		*name,
	uint		form,
	uint		type,
	uint		flags)
{
	fs_project_t	*p;
	prid_t		id;

	if (!name) {
		exitcode = 1;
		fprintf(stderr, _("%s: must specify a project name/ID\n"),
			progname);
		return;
	}

	if (isdigit(name[0])) {
		id = atoi(name);
		name = getprojectname(id, flags & NO_LOOKUP_FLAG);
	} else if ((p = getprnam(name))) {
		id = p->pr_prid;
		name = p->pr_name;
	} else {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot find project %s\n"),
			progname, name);
		return;
	}

	quota(fp, id, name, form, type, flags);
}

static void
quota_any_type(
	FILE		*fp,
	char		*name,
	uint		form,
	uint		type,
	uint		flags)
{
	switch (type) {
	case XFS_USER_QUOTA:
		quota_user_type(fp, name, form, type, flags);
		break;
	case XFS_GROUP_QUOTA:
		quota_group_type(fp, name, form, type, flags);
		break;
	case XFS_PROJ_QUOTA:
		quota_proj_type(fp, name, form, type, flags);
		break;
	}
}

static int
quota_f(
	int		argc,
	char		**argv)
{
	FILE		*fp = NULL;
	char		*fname = NULL;
	int		c, flags = 0, type = 0, form = 0;

	while ((c = getopt(argc, argv, "bf:ghnNipruv")) != EOF) {
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
			type = XFS_GROUP_QUOTA;
			break;
		case 'p':
			type = XFS_PROJ_QUOTA;
			break;
		case 'u':
			type = XFS_USER_QUOTA;
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
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&quota_cmd);
		}
	}

	if (!form)
		form = XFS_BLOCK_QUOTA;

	if (!type)
		type = XFS_USER_QUOTA;

	if ((fp = fopen_write_secure(fname)) == NULL)
		return 0;

	if (argc == optind)
		quota_any_type(fp, NULL, form, type, flags);
	else while (argc > optind)
		quota_any_type(fp, argv[optind++], form, type, flags);

	if (fname)
		fclose(fp);
	return 0;
}

void
quota_init(void)
{
	quota_cmd.name = "quota";
	quota_cmd.altname = "l";
	quota_cmd.cfunc = quota_f;
	quota_cmd.argmin = 0;
	quota_cmd.argmax = -1;
	quota_cmd.args = _("[-bir] [-gpu] [-hnNv] [-f file] [id|name]...");
	quota_cmd.oneline = _("show usage and limits");
	quota_cmd.help = quota_help;

	add_command(&quota_cmd);
}
