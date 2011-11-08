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
#include <xfs/input.h>
#include "init.h"
#include "quota.h"

static cmdinfo_t project_cmd;
static prid_t prid;
static int recurse_depth = -1;

enum {
	CHECK_PROJECT	= 0x1,
	SETUP_PROJECT	= 0x2,
	CLEAR_PROJECT	= 0x4,
};

#define EXCLUDED_FILE_TYPES(x) \
	   (S_ISCHR((x)) \
	|| S_ISBLK((x)) \
	|| S_ISFIFO((x)) \
	|| S_ISLNK((x)) \
	|| S_ISSOCK((x)))

static void
project_help(void)
{
	printf(_(
"\n"
" list projects or setup a project tree for tree quota management\n"
"\n"
" Example:\n"
" 'project -c logfiles'\n"
" (match project 'logfiles' to a directory, and setup the directory tree)\n"
"\n"
" Without arguments, report all projects found in the /etc/projects file.\n"
" The project quota mechanism in XFS can be used to implement a form of\n"
" directory tree quota, where a specified directory and all of the files\n"
" and subdirectories below it (i.e. a tree) can be restricted to using a\n"
" subset of the available space in the filesystem.\n"
"\n"
" A managed tree must be setup initially using the -c option with a project.\n"
" The specified project name or identifier is matched to one or more trees\n"
" defined in /etc/projects, and these trees are then recursively descended\n"
" to mark the affected inodes as being part of that tree - which sets inode\n"
" flags and the project identifier on every file.\n"
" Once this has been done, new files created in the tree will automatically\n"
" be accounted to the tree based on their project identifier.  An attempt to\n"
" create a hard link to a file in the tree will only succeed if the project\n"
" identifier matches the project identifier for the tree.  The xfs_io utility\n"
" can be used to set the project ID for an arbitrary file, but this can only\n"
" be done by a privileged user.\n"
"\n"
" A previously setup tree can be cleared from project quota control through\n"
" use of the -C option, which will recursively descend the tree, clearing\n"
" the affected inodes from project quota control.\n"
"\n"
" The -c option can be used to check whether a tree is setup, it reports\n"
" nothing if the tree is correct, otherwise it reports the paths of inodes\n"
" which do not have the project ID of the rest of the tree, or if the inode\n"
" flag is not set.\n"
"\n"
" The -p <path> option can be used to manually specify project path without\n"
" need to create /etc/projects file. This option can be used multiple times\n"
" to specify multiple paths. When using this option only one projid/name can\n"
" be specified at command line. Note that /etc/projects is also used if exists.\n"
"\n"
" The -d <depth> option allows to descend at most <depth> levels of directories\n"
" below the command line arguments. -d 0 means only apply the actions\n"
" to the top level of the projects. -d -1 means no recursion limit (default).\n"
"\n"
" The /etc/projid and /etc/projects file formats are simple, and described\n"
" on the xfs_quota man page.\n"
"\n"));
}

static int
check_project(
	const char		*path,
	const struct stat	*stat,
	int			flag,
	struct FTW		*data)
{
	struct fsxattr		fsx;
	int			fd;

	if (recurse_depth >= 0 && data->level > recurse_depth)
		return -1;

	if (flag == FTW_NS ){
		exitcode = 1;
		fprintf(stderr, _("%s: cannot stat file %s\n"), progname, path);
		return 0;
	}
	if (EXCLUDED_FILE_TYPES(stat->st_mode)) {
		fprintf(stderr, _("%s: skipping special file %s\n"), progname, path);
		return 0;
	}

	if ((fd = open(path, O_RDONLY|O_NOCTTY)) == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else if ((xfsctl(path, fd, XFS_IOC_FSGETXATTR, &fsx)) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		if (fsx.fsx_projid != prid)
			printf(_("%s - project identifier is not set"
				 " (inode=%u, tree=%u)\n"),
				path, fsx.fsx_projid, (unsigned int)prid);
		if (!(fsx.fsx_xflags & XFS_XFLAG_PROJINHERIT))
			printf(_("%s - project inheritance flag is not set\n"),
				path);
	}
	if (fd != -1)
		close(fd);
	return 0;
}

static int
clear_project(
	const char		*path,
	const struct stat	*stat,
	int			flag,
	struct FTW		*data)
{
	struct fsxattr		fsx;
	int			fd;

	if (recurse_depth >= 0 && data->level > recurse_depth)
		return -1;

	if (flag == FTW_NS ){
		exitcode = 1;
		fprintf(stderr, _("%s: cannot stat file %s\n"), progname, path);
		return 0;
	}
	if (EXCLUDED_FILE_TYPES(stat->st_mode)) {
		fprintf(stderr, _("%s: skipping special file %s\n"), progname, path);
		return 0;
	}

	if ((fd = open(path, O_RDONLY|O_NOCTTY)) == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
		return 0;
	} else if (xfsctl(path, fd, XFS_IOC_FSGETXATTR, &fsx) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, path, strerror(errno));
		close(fd);
		return 0;
	}

	fsx.fsx_projid = 0;
	fsx.fsx_xflags &= ~XFS_XFLAG_PROJINHERIT;
	if (xfsctl(path, fd, XFS_IOC_FSSETXATTR, &fsx) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot clear project on %s: %s\n"),
			progname, path, strerror(errno));
	}
	close(fd);
	return 0;
}

static int
setup_project(
	const char		*path,
	const struct stat	*stat,
	int			flag,
	struct FTW		*data)
{
	struct fsxattr		fsx;
	int			fd;

	if (recurse_depth >= 0 && data->level > recurse_depth)
		return -1;

	if (flag == FTW_NS ){
		exitcode = 1;
		fprintf(stderr, _("%s: cannot stat file %s\n"), progname, path);
		return 0;
	}
	if (EXCLUDED_FILE_TYPES(stat->st_mode)) {
		fprintf(stderr, _("%s: skipping special file %s\n"), progname, path);
		return 0;
	}

	if ((fd = open(path, O_RDONLY|O_NOCTTY)) == -1) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
		return 0;
	} else if (xfsctl(path, fd, XFS_IOC_FSGETXATTR, &fsx) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, path, strerror(errno));
		close(fd);
		return 0;
	}

	fsx.fsx_projid = prid;
	fsx.fsx_xflags |= XFS_XFLAG_PROJINHERIT;
	if (xfsctl(path, fd, XFS_IOC_FSSETXATTR, &fsx) < 0) {
		exitcode = 1;
		fprintf(stderr, _("%s: cannot set project on %s: %s\n"),
			progname, path, strerror(errno));
	}
	close(fd);
	return 0;
}

static void
project_operations(
	char		*project,
	char		*dir,
	int		type)
{
	switch (type) {
	case CHECK_PROJECT:
		printf(_("Checking project %s (path %s)...\n"), project, dir);
		nftw(dir, check_project, 100, FTW_PHYS|FTW_MOUNT);
		break;
	case SETUP_PROJECT:
		printf(_("Setting up project %s (path %s)...\n"), project, dir);
		nftw(dir, setup_project, 100, FTW_PHYS|FTW_MOUNT);
		break;
	case CLEAR_PROJECT:
		printf(_("Clearing project %s (path %s)...\n"), project, dir);
		nftw(dir, clear_project, 100, FTW_PHYS|FTW_MOUNT);
		break;
	}
}

static void
project(
	char		*project,
	int		type)
{
	fs_cursor_t     cursor;
	fs_path_t	*path;
	int		count = 0;

	fs_cursor_initialise(NULL, FS_PROJECT_PATH, &cursor);
	while ((path = fs_cursor_next_entry(&cursor))) {
		if (prid != path->fs_prid && path->fs_prid != -1)
			continue;
		project_operations(project, path->fs_dir, type);
		count++;
	}

	printf(_("Processed %d (%s and cmdline) paths for project %s with "
		"recursion depth %s (%d).\n"),
		 count, projects_file, project,
		 recurse_depth < 0 ? _("infinite") : _("limited"), recurse_depth);
}

static int
project_f(
	int		argc,
	char		**argv)
{
	int		c, type = 0, ispath = 0;

	while ((c = getopt(argc, argv, "cd:p:sC")) != EOF) {
		switch (c) {
		case 'c':
			type = CHECK_PROJECT;
			break;
		case 'd':
			recurse_depth = atoi(optarg);
			if (recurse_depth < 0)
				recurse_depth = -1;
			break;
		case 'p':
			ispath = 1;
			fs_table_insert_project_path(optarg, -1);
			break;
		case 's':
			type = SETUP_PROJECT;
			break;
		case 'C':
			type = CLEAR_PROJECT;
			break;
		default:
			return command_usage(&project_cmd);
		}
	}

	if (argc == optind)
		return command_usage(&project_cmd);

	/* no options - just check the given projects */
	if (!type)
		type = CHECK_PROJECT;

	setprfiles();
	if (!ispath && access(projects_file, F_OK) != 0) {
		exitcode = 1;
		fprintf(stderr, _("projects file \"%s\" doesn't exist\n"),
			projects_file);
		return 0;
	}

	if (ispath && argc - optind > 1) {
		exitcode = 1;
		fprintf(stderr, _("%s: only one projid/name can be specified "
			    "when using -p <path>, %d found.\n"),
			progname, argc - optind);
		return 0;
	}

        while (argc > optind) {
		prid = prid_from_string(argv[optind]);
		if (prid == -1) {
			exitcode = 1;
			fprintf(stderr, _("%s - no such project in %s "
				    "or invalid project number\n"),
				argv[optind], projects_file);
		} else
	                project(argv[optind], type);
		optind++;
	}

	return 0;
}

void
project_init(void)
{
	project_cmd.name = "project";
	project_cmd.altname = "tree";
	project_cmd.cfunc = project_f;
	project_cmd.args = _("[-c|-s|-C|-d <depth>|-p <path>] project ...");
	project_cmd.argmin = 1;
	project_cmd.argmax = -1;
	project_cmd.oneline = _("check, setup or clear project quota trees");
	project_cmd.help = project_help;

	if (expert)
		add_command(&project_cmd);
}
