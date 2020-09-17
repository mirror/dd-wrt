// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
/*
 * If configure didn't find a struct fsxattr with fsx_cowextsize,
 * disable the only other source (so far) of struct fsxattr.  Thus,
 * build with the internal definition of struct fsxattr, which has
 * fsx_cowextsize.
 */
#include "platform_defs.h"
#include "command.h"
#include "init.h"
#include "io.h"
#include "input.h"
#include "libfrog/paths.h"

static cmdinfo_t cowextsize_cmd;
static long cowextsize;

static void
cowextsize_help(void)
{
	printf(_(
"\n"
" report or modify preferred CoW extent size (in bytes) for the current path\n"
"\n"
" -R -- recursively descend (useful when current path is a directory)\n"
" -D -- recursively descend, only modifying cowextsize on directories\n"
"\n"));
}

static int
get_cowextsize(const char *path, int fd)
{
	struct fsxattr	fsx;

	if ((xfsctl(path, fd, FS_IOC_FSGETXATTR, &fsx)) < 0) {
		printf("%s: XFS_IOC_FSGETXATTR %s: %s\n",
			progname, path, strerror(errno));
		exitcode = 1;
		return 0;
	}
	printf("[%u] %s\n", fsx.fsx_cowextsize, path);
	return 0;
}

static int
set_cowextsize(const char *path, int fd, long extsz)
{
	struct fsxattr	fsx;
	struct stat64	stat;

	if (fstat64(fd, &stat) < 0) {
		perror("fstat64");
		exitcode = 1;
		return 0;
	}
	if ((xfsctl(path, fd, FS_IOC_FSGETXATTR, &fsx)) < 0) {
		printf("%s: XFS_IOC_FSGETXATTR %s: %s\n",
			progname, path, strerror(errno));
		exitcode = 1;
		return 0;
	}

	if (S_ISREG(stat.st_mode) || S_ISDIR(stat.st_mode)) {
		fsx.fsx_xflags |= FS_XFLAG_COWEXTSIZE;
	} else {
		printf(_("invalid target file type - file %s\n"), path);
		return 0;
	}
	fsx.fsx_cowextsize = extsz;

	if ((xfsctl(path, fd, FS_IOC_FSSETXATTR, &fsx)) < 0) {
		printf("%s: XFS_IOC_FSSETXATTR %s: %s\n",
			progname, path, strerror(errno));
		exitcode = 1;
		return 0;
	}

	return 0;
}

static int
get_cowextsize_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		get_cowextsize(path, fd);
		close(fd);
	}
	return 0;
}

static int
set_cowextsize_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		set_cowextsize(path, fd, cowextsize);
		close(fd);
	}
	return 0;
}

static int
cowextsize_f(
	int		argc,
	char		**argv)
{
	size_t			blocksize, sectsize;
	int			c;

	recurse_all = recurse_dir = 0;
	init_cvtnum(&blocksize, &sectsize);
	while ((c = getopt(argc, argv, "DR")) != EOF) {
		switch (c) {
		case 'D':
			recurse_dir = 1;
			break;
		case 'R':
			recurse_all = 1;
			break;
		default:
			return command_usage(&cowextsize_cmd);
		}
	}

	if (optind < argc) {
		cowextsize = (long)cvtnum(blocksize, sectsize, argv[optind]);
		if (cowextsize < 0) {
			printf(_("non-numeric cowextsize argument -- %s\n"),
				argv[optind]);
			exitcode = 1;
			return 0;
		}
	} else {
		cowextsize = -1;
	}

	if (recurse_all && recurse_dir) {
		fprintf(stderr, _("%s: -R and -D options are mutually exclusive\n"),
			progname);
		exitcode = 1;
		return 0;
	}

	if (recurse_all || recurse_dir)
		nftw(file->name, (cowextsize >= 0) ?
			set_cowextsize_callback : get_cowextsize_callback,
			100, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
	else if (cowextsize >= 0)
		set_cowextsize(file->name, file->fd, cowextsize);
	else
		get_cowextsize(file->name, file->fd);
	return 0;
}

void
cowextsize_init(void)
{
	cowextsize_cmd.name = "cowextsize";
	cowextsize_cmd.cfunc = cowextsize_f;
	cowextsize_cmd.args = _("[-D | -R] [cowextsize]");
	cowextsize_cmd.argmin = 0;
	cowextsize_cmd.argmax = -1;
	cowextsize_cmd.flags = CMD_NOMAP_OK;
	cowextsize_cmd.oneline =
		_("get/set preferred CoW extent size (in bytes) for the open file");
	cowextsize_cmd.help = cowextsize_help;

	add_command(&cowextsize_cmd);
}
