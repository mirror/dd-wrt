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
#include "init.h"
#include "quota.h"

static cmdinfo_t off_cmd;
static cmdinfo_t state_cmd;
static cmdinfo_t enable_cmd;
static cmdinfo_t disable_cmd;
static cmdinfo_t remove_cmd;

static void
off_help(void)
{
	printf(_(
"\n"
" turn filesystem quota off, both accounting and enforcement\n"
"\n"
" Example:\n"
" 'off -uv'  (switch off user quota on the current filesystem)\n"
" This command is the equivalent of the traditional quotaoff command,\n"
" which disables quota completely on a mounted filesystem.\n"
" Note that there is no 'on' command - for XFS filesystems (with the\n"
" exception of the root filesystem on IRIX) quota can only be enabled\n"
" at mount time, through the use of one of the quota mount options.\n"
"\n"
" The state command is useful for displaying the current state.  Using\n"
" the -v (verbose) option with the 'off' command will display the quota\n"
" state for the affected filesystem once the operation is complete.\n"
" The affected quota type is -g (groups), -p (projects) or -u (users)\n"
" and defaults to user quota (multiple types can be specified).\n"
"\n"));
}

static void
state_help(void)
{
	printf(_(
"\n"
" query the state of quota on the current filesystem\n"
"\n"
" This is a verbose status command, reporting whether or not accounting\n"
" and/or enforcement are enabled for a filesystem, which inodes are in\n"
" use as the quota state inodes, and how many extents and blocks are\n"
" presently being used to hold that information.\n"
" The quota type is specified via -g (groups), -p (projects) or -u (users)\n"
" and defaults to user quota (multiple types can be specified).\n"
"\n"));
}

static void
enable_help(void)
{
	printf(_(
"\n"
" enable quota enforcement on a filesystem\n"
"\n"
" If a filesystem is mounted and has quota accounting enabled, but not\n"
" quota enforcement, enforcement can be enabled with this command.\n"
" With the -v (verbose) option, the status of the filesystem will be\n"
" reported after the operation is complete.\n"
" The affected quota type is -g (groups), -p (projects) or -u (users)\n"
" and defaults to user quota (multiple types can be specified).\n"
"\n"));
}

static void
disable_help(void)
{
	printf(_(
"\n"
" disable quota enforcement on a filesystem\n"
"\n"
" If a filesystem is mounted and is currently enforcing quota, this\n"
" provides a mechanism to switch off the enforcement, but continue to\n"
" perform used space (and used inodes) accounting.\n"
" The affected quota type is -g (groups), -p (projects) or -u (users).\n"
"\n"));
}

static void
remove_help(void)
{
	printf(_(
"\n"
" remove any space being used by the quota subsystem\n"
"\n"
" Once quota has been switched 'off' on a filesystem, the space that\n"
" was allocated to holding quota metadata can be freed via this command.\n"
" The affected quota type is -g (groups), -p (projects) or -u (users)\n"
" and defaults to user quota (multiple types can be specified).\n"
"\n"));
}

static void
state_qfilestat(
	FILE		*fp,
	fs_path_t	*mount,
	uint		type,
	fs_qfilestat_t	*qfs,
	int		accounting,
	int		enforcing)
{
	fprintf(fp, _("%s quota state on %s (%s)\n"), type_to_string(type),
		mount->fs_dir, mount->fs_name);
	fprintf(fp, _("  Accounting: %s\n"), accounting ? _("ON") : _("OFF"));
	fprintf(fp, _("  Enforcement: %s\n"), enforcing ? _("ON") : _("OFF"));
	if (qfs->qfs_ino != (__u64) -1)
		fprintf(fp, _("  Inode: #%llu (%llu blocks, %lu extents)\n"),
			(unsigned long long)qfs->qfs_ino,
			(unsigned long long)qfs->qfs_nblks,
			(unsigned long)qfs->qfs_nextents);
	else
		fprintf(fp, _("  Inode: N/A\n"));
}

static void
state_timelimit(
	FILE		*fp,
	uint		form,
	__uint32_t	timelimit)
{
	fprintf(fp, _("%s grace time: %s\n"),
		form_to_string(form),
		time_to_string(timelimit, VERBOSE_FLAG | ABSOLUTE_FLAG));
}

static void
state_quotafile_mount(
	FILE		*fp,
	uint		type,
	fs_path_t	*mount,
	uint		flags)
{
	fs_quota_stat_t	s;
	char		*dev = mount->fs_name;

	if (xfsquotactl(XFS_GETQSTAT, dev, type, 0, (void *)&s) < 0) {
		if (flags & VERBOSE_FLAG)
			fprintf(fp, _("%s quota are not enabled on %s\n"),
				type_to_string(type), dev);
		return;
	}

	if (type & XFS_USER_QUOTA)
		state_qfilestat(fp, mount, XFS_USER_QUOTA, &s.qs_uquota,
				s.qs_flags & XFS_QUOTA_UDQ_ACCT,
				s.qs_flags & XFS_QUOTA_UDQ_ENFD);
	if (type & XFS_GROUP_QUOTA)
		state_qfilestat(fp, mount, XFS_GROUP_QUOTA, &s.qs_gquota,
				s.qs_flags & XFS_QUOTA_GDQ_ACCT,
				s.qs_flags & XFS_QUOTA_GDQ_ENFD);
	if (type & XFS_PROJ_QUOTA)
		state_qfilestat(fp, mount, XFS_PROJ_QUOTA, &s.qs_gquota,
				s.qs_flags & XFS_QUOTA_PDQ_ACCT,
				s.qs_flags & XFS_QUOTA_PDQ_ENFD);

	state_timelimit(fp, XFS_BLOCK_QUOTA, s.qs_btimelimit);
	state_timelimit(fp, XFS_INODE_QUOTA, s.qs_itimelimit);
	state_timelimit(fp, XFS_RTBLOCK_QUOTA, s.qs_rtbtimelimit);
}

static void
state_quotafile(
	FILE		*fp,
	uint		type,
	char		*dir,
	uint		flags)
{
	fs_cursor_t	cursor;
	fs_path_t	*mount;

	fs_cursor_initialise(dir, FS_MOUNT_POINT, &cursor);
	while ((mount = fs_cursor_next_entry(&cursor)))
		state_quotafile_mount(fp, type, mount, flags);
}

static int
state_f(
	int		argc,
	char		**argv)
{
	FILE		*fp = NULL;
	char		*fname = NULL;
	int		c, flags = 0, type = 0;

	while ((c = getopt(argc, argv, "af:gpuv")) != EOF) {
		switch (c) {
		case 'a':
			flags |= ALL_MOUNTS_FLAG;
			break;
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
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&state_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&state_cmd);

	if ((fp = fopen_write_secure(fname)) == NULL)
		return 0;

	if (!type)
		type = XFS_USER_QUOTA | XFS_GROUP_QUOTA | XFS_PROJ_QUOTA;

	if (flags & ALL_MOUNTS_FLAG)
		state_quotafile(fp, type, NULL, flags);
	else if (fs_path && fs_path->fs_flags & FS_MOUNT_POINT)
		state_quotafile(fp, type, fs_path->fs_dir, flags);

	if (fname)
		fclose(fp);
	return 0;
}

static void
enable_enforcement(
	char		*dir,
	uint		type,
	uint		qflags,
	uint		flags)
{
	fs_path_t	*mount;

	mount = fs_table_lookup(dir, FS_MOUNT_POINT);
	if (!mount) {
		exitcode = 1;
		fprintf(stderr, "%s: unknown mount point %s\n", progname, dir);
		return;
	}
	dir = mount->fs_name;
	if (xfsquotactl(XFS_QUOTAON, dir, type, 0, (void *)&qflags) < 0)
		perror("XFS_QUOTAON");
	else if (flags & VERBOSE_FLAG)
		state_quotafile_mount(stdout, type, mount, flags);
}

static void
disable_enforcement(
	char		*dir,
	uint		type,
	uint		qflags,
	uint		flags)
{
	fs_path_t	*mount;

	mount = fs_table_lookup(dir, FS_MOUNT_POINT);
	if (!mount) {
		exitcode = 1;
		fprintf(stderr, "%s: unknown mount point %s\n", progname, dir);
		return;
	}
	dir = mount->fs_name;
	if (xfsquotactl(XFS_QUOTAOFF, dir, type, 0, (void *)&qflags) < 0)
		perror("XFS_QUOTAOFF");
	else if (flags & VERBOSE_FLAG)
		state_quotafile_mount(stdout, type, mount, flags);
}

static void
quotaoff(
	char		*dir,
	uint		type,
	uint		qflags,
	uint		flags)
{
	fs_path_t	*mount;

	mount = fs_table_lookup(dir, FS_MOUNT_POINT);
	if (!mount) {
		exitcode = 1;
		fprintf(stderr, "%s: unknown mount point %s\n", progname, dir);
		return;
	}
	dir = mount->fs_name;
	if (xfsquotactl(XFS_QUOTAOFF, dir, type, 0, (void *)&qflags) < 0)
		perror("XFS_QUOTAOFF");
	else if (flags & VERBOSE_FLAG)
		state_quotafile_mount(stdout, type, mount, flags);
}

static int
remove_qtype_extents(
	char		*dir,
	uint		type)
{
	int	error = 0;

	if ((error = xfsquotactl(XFS_QUOTARM, dir, type, 0, (void *)&type)) < 0)
		perror("XFS_QUOTARM");
	return error;
}

static void
remove_extents(
	char		*dir,
	uint		type,
	uint		flags)
{
	fs_path_t	*mount;

	mount = fs_table_lookup(dir, FS_MOUNT_POINT);
	if (!mount) {
		exitcode = 1;
		fprintf(stderr, "%s: unknown mount point %s\n", progname, dir);
		return;
	}
	dir = mount->fs_name;
	if (type & XFS_USER_QUOTA) {
		if (remove_qtype_extents(dir, XFS_USER_QUOTA) < 0) 
			return;
	}
	if (type & XFS_GROUP_QUOTA) {
		if (remove_qtype_extents(dir, XFS_GROUP_QUOTA) < 0) 
			return;
	} else if (type & XFS_PROJ_QUOTA) {
		if (remove_qtype_extents(dir, XFS_PROJ_QUOTA) < 0) 
			return;
	}
	if (flags & VERBOSE_FLAG)
		state_quotafile_mount(stdout, type, mount, flags);
}

static int
enable_f(
	int		argc,
	char		**argv)
{
	int		c, flags = 0, qflags = 0, type = 0;

	while ((c = getopt(argc, argv, "gpuv")) != EOF) {
		switch (c) {
		case 'g':
			type |= XFS_GROUP_QUOTA;
			qflags |= XFS_QUOTA_GDQ_ACCT | XFS_QUOTA_GDQ_ENFD;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			qflags |= XFS_QUOTA_PDQ_ACCT | XFS_QUOTA_PDQ_ENFD;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			qflags |= XFS_QUOTA_UDQ_ACCT | XFS_QUOTA_UDQ_ENFD;
			break;
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&enable_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&enable_cmd);

	if (!type) {
		type |= XFS_USER_QUOTA;
		qflags |= XFS_QUOTA_UDQ_ACCT | XFS_QUOTA_UDQ_ENFD;
	}

	if (fs_path->fs_flags & FS_MOUNT_POINT)
		enable_enforcement(fs_path->fs_dir, type, qflags, flags);
	return 0;
}

static int
disable_f(
	int		argc,
	char		**argv)
{
	int		c, flags = 0, qflags = 0, type = 0;

	while ((c = getopt(argc, argv, "gpuv")) != EOF) {
		switch (c) {
		case 'g':
			type |= XFS_GROUP_QUOTA;
			qflags |= XFS_QUOTA_GDQ_ENFD;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			qflags |= XFS_QUOTA_PDQ_ENFD;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			qflags |= XFS_QUOTA_UDQ_ENFD;
			break;
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&disable_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&disable_cmd);

	if (!type) {
		type |= XFS_USER_QUOTA;
		qflags |= XFS_QUOTA_UDQ_ENFD;
	}

	if (fs_path->fs_flags & FS_MOUNT_POINT)
		disable_enforcement(fs_path->fs_dir, type, qflags, flags);
	return 0;
}

static int
off_f(
	int		argc,
	char		**argv)
{
	int		c, flags = 0, qflags = 0, type = 0;

	while ((c = getopt(argc, argv, "gpuv")) != EOF) {
		switch (c) {
		case 'g':
			type |= XFS_GROUP_QUOTA;
			qflags |= XFS_QUOTA_GDQ_ACCT | XFS_QUOTA_GDQ_ENFD;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			qflags |= XFS_QUOTA_PDQ_ACCT | XFS_QUOTA_PDQ_ENFD;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			qflags |= XFS_QUOTA_UDQ_ACCT | XFS_QUOTA_UDQ_ENFD;
			break;
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&off_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&off_cmd);

	if (!type) {
		type |= XFS_USER_QUOTA;
		qflags |= XFS_QUOTA_UDQ_ACCT | XFS_QUOTA_UDQ_ENFD;
	}

	if (fs_path->fs_flags & FS_MOUNT_POINT)
		quotaoff(fs_path->fs_dir, type, qflags, flags);
	return 0;
}

static int
remove_f(
	int		argc,
	char		**argv)
{
	int		c, flags = 0, type = 0;

	while ((c = getopt(argc, argv, "gpuv")) != EOF) {
		switch (c) {
		case 'g':
			type |= XFS_GROUP_QUOTA;
			break;
		case 'p':
			type |= XFS_PROJ_QUOTA;
			break;
		case 'u':
			type |= XFS_USER_QUOTA;
			break;
		case 'v':
			flags |= VERBOSE_FLAG;
			break;
		default:
			return command_usage(&remove_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&remove_cmd);

	if (!type) {
		type |= XFS_USER_QUOTA;
	}

	if (fs_path->fs_flags & FS_MOUNT_POINT)
		remove_extents(fs_path->fs_dir, type, flags);
	return 0;
}

void
state_init(void)
{
	off_cmd.name = "off";
	off_cmd.cfunc = off_f;
	off_cmd.argmin = 0;
	off_cmd.argmax = -1;
	off_cmd.args = _("[-gpu] [-v]");
	off_cmd.oneline = _("permanently switch quota off for a path");
	off_cmd.help = off_help;

	state_cmd.name = "state";
	state_cmd.cfunc = state_f;
	state_cmd.argmin = 0;
	state_cmd.argmax = -1;
	state_cmd.args = _("[-gpu] [-a] [-v] [-f file]");
	state_cmd.oneline = _("get overall quota state information");
	state_cmd.help = state_help;

	enable_cmd.name = "enable";
	enable_cmd.cfunc = enable_f;
	enable_cmd.argmin = 0;
	enable_cmd.argmax = -1;
	enable_cmd.args = _("[-gpu] [-v]");
	enable_cmd.oneline = _("enable quota enforcement");
	enable_cmd.help = enable_help;

	disable_cmd.name = "disable";
	disable_cmd.cfunc = disable_f;
	disable_cmd.argmin = 0;
	disable_cmd.argmax = -1;
	disable_cmd.args = _("[-gpu] [-v]");
	disable_cmd.oneline = _("disable quota enforcement");
	disable_cmd.help = disable_help;

	remove_cmd.name = "remove";
	remove_cmd.cfunc = remove_f;
	remove_cmd.argmin = 0;
	remove_cmd.argmax = -1;
	remove_cmd.args = _("[-gpu] [-v]");
	remove_cmd.oneline = _("remove quota extents from a filesystem");
	remove_cmd.help = remove_help;

	if (expert) {
		add_command(&off_cmd);
		add_command(&state_cmd);
		add_command(&enable_cmd);
		add_command(&disable_cmd);
		add_command(&remove_cmd);
	}
}
