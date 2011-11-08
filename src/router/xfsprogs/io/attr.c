/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
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

#include <xfs/xfs.h>
#include <xfs/command.h>
#include <xfs/input.h>
#include "init.h"
#include "io.h"

static cmdinfo_t chattr_cmd;
static cmdinfo_t lsattr_cmd;
static unsigned int orflags;
static unsigned int andflags;
unsigned int recurse_all;
unsigned int recurse_dir;

static struct xflags {
	uint	flag;
	char	*shortname;
	char	*longname;
} xflags[] = {
	{ XFS_XFLAG_REALTIME,		"r", "realtime"		},
	{ XFS_XFLAG_PREALLOC,		"p", "prealloc"		},
	{ XFS_XFLAG_IMMUTABLE,		"i", "immutable"	},
	{ XFS_XFLAG_APPEND,		"a", "append-only"	},
	{ XFS_XFLAG_SYNC,		"s", "sync"		},
	{ XFS_XFLAG_NOATIME,		"A", "no-atime"		},
	{ XFS_XFLAG_NODUMP,		"d", "no-dump"		},
	{ XFS_XFLAG_RTINHERIT,		"t", "rt-inherit"	},
	{ XFS_XFLAG_PROJINHERIT,	"P", "proj-inherit"	},
	{ XFS_XFLAG_NOSYMLINKS,		"n", "nosymlinks"	},
	{ XFS_XFLAG_EXTSIZE,		"e", "extsize"		},
	{ XFS_XFLAG_EXTSZINHERIT,	"E", "extsz-inherit"	},
	{ XFS_XFLAG_NODEFRAG,		"f", "no-defrag"	},
	{ XFS_XFLAG_FILESTREAM,		"S", "filestream"	},
	{ 0, NULL, NULL }
};
#define CHATTR_XFLAG_LIST	"r"/*p*/"iasAdtPneEfS"

static void
lsattr_help(void)
{
	printf(_(
"\n"
" displays the set of extended inode flags associated with the current file\n"
"\n"
" Each individual flag is displayed as a single character, in this order:\n"
" r -- file data is stored in the realtime section\n"
" p -- file has preallocated extents (cannot be changed using chattr)\n"
" i -- immutable, file cannot be modified\n"
" a -- append-only, file can only be appended to\n"
" s -- all updates are synchronous\n"
" A -- the access time is not updated for this inode\n"
" d -- do not include this file in a dump of the filesystem\n"
" t -- child created in this directory has realtime bit set by default\n"
" P -- child created in this directory has parents project ID by default\n"
" n -- symbolic links cannot be created in this directory\n"
" e -- for non-realtime files, observe the inode extent size value\n"
" E -- children created in this directory inherit the extent size value\n"
" f -- do not include this file when defragmenting the filesystem\n"
" S -- enable filestreams allocator for this directory\n"
"\n"
" Options:\n"
" -R -- recursively descend (useful when current file is a directory)\n"
" -D -- recursively descend, but only list attributes on directories\n"
" -a -- show all flags which can be set alongside those which are set\n"
" -v -- verbose mode; show long names of flags, not single characters\n"
"\n"));
}

static void
chattr_help(void)
{
	printf(_(
"\n"
" modifies the set of extended inode flags associated with the current file\n"
"\n"
" Examples:\n"
" 'chattr +a' - sets the append-only flag\n"
" 'chattr -a' - clears the append-only flag\n"
"\n"
" -R -- recursively descend (useful when current file is a directory)\n"
" -D -- recursively descend, only modifying attributes on directories\n"
" +/-r -- set/clear the realtime flag\n"
" +/-i -- set/clear the immutable flag\n"
" +/-a -- set/clear the append-only flag\n"
" +/-s -- set/clear the sync flag\n"
" +/-A -- set/clear the no-atime flag\n"
" +/-d -- set/clear the no-dump flag\n"
" +/-t -- set/clear the realtime inheritance flag\n"
" +/-P -- set/clear the project ID inheritance flag\n"
" +/-n -- set/clear the no-symbolic-links flag\n"
" +/-e -- set/clear the extent-size flag\n"
" +/-E -- set/clear the extent-size inheritance flag\n"
" +/-f -- set/clear the no-defrag flag\n"
" +/-S -- set/clear the filestreams allocator flag\n"
" Note1: user must have certain capabilities to modify immutable/append-only.\n"
" Note2: immutable/append-only files cannot be deleted; removing these files\n"
"        requires the immutable/append-only flag to be cleared first.\n"
" Note3: the realtime flag can only be set if the filesystem has a realtime\n"
"        section, and the (regular) file must be empty when the flag is set.\n"
"\n"));
}

void
printxattr(
	uint		flags,
	int		verbose,
	int		dofname,
	const char	*fname,
	int		dobraces,
	int		doeol)
{
	struct xflags	*p;
	int		first = 1;

	if (dobraces)
		fputs("[", stdout);
	for (p = xflags; p->flag; p++) {
		if (flags & p->flag) {
			if (verbose) {
				if (first)
					first = 0;
				else
					fputs(", ", stdout);
				fputs(p->longname, stdout);
			} else {
				fputs(p->shortname, stdout);
			}
		} else if (!verbose) {
			fputs("-", stdout);
		}
	}
	if (dobraces)
		fputs("]", stdout);
	if (dofname)
		printf(" %s ", fname);
	if (doeol)
		fputs("\n", stdout);
}

static int
lsattr_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	struct fsxattr		fsx;
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	if ((fd = open(path, O_RDONLY)) == -1)
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	else if ((xfsctl(path, fd, XFS_IOC_FSGETXATTR, &fsx)) < 0)
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, path, strerror(errno));
	else
		printxattr(fsx.fsx_xflags, 0, 1, path, 0, 1);

	if (fd != -1)
		close(fd);
	return 0;
}

static int
lsattr_f(
	int		argc,
	char		**argv)
{
	struct fsxattr	fsx;
	char		*name = file->name;
	int		c, aflag = 0, vflag = 0;

	recurse_all = recurse_dir = 0;
	while ((c = getopt(argc, argv, "DRav")) != EOF) {
		switch (c) {
		case 'D':
			recurse_all = 0;
			recurse_dir = 1;
			break;
		case 'R':
			recurse_all = 1;
			recurse_dir = 0;
			break;
		case 'a':
			aflag = 1;
			vflag = 0;
			break;
		case 'v':
			aflag = 0;
			vflag = 1;
			break;
		default:
			return command_usage(&lsattr_cmd);
		}
	}

	if (recurse_all || recurse_dir) {
		nftw(name, lsattr_callback,
			100, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
	} else if ((xfsctl(name, file->fd, XFS_IOC_FSGETXATTR, &fsx)) < 0) {
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, name, strerror(errno));
	} else {
		printxattr(fsx.fsx_xflags, vflag, !aflag, name, vflag, !aflag);
		if (aflag) {
			fputs("/", stdout);
			printxattr(-1, 0, 1, name, 0, 1);
		}
	}
	return 0;
}

static int
chattr_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	struct fsxattr		attr;
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	if ((fd = open(path, O_RDONLY)) == -1) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else if (xfsctl(path, fd, XFS_IOC_FSGETXATTR, &attr) < 0) {
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		attr.fsx_xflags |= orflags;
		attr.fsx_xflags &= ~andflags;
		if (xfsctl(path, fd, XFS_IOC_FSSETXATTR, &attr) < 0)
			fprintf(stderr, _("%s: cannot set flags on %s: %s\n"),
				progname, path, strerror(errno));
	}

	if (fd != -1)
		close(fd);
	return 0;
}

static int
chattr_f(
	int		argc,
	char		**argv)
{
	struct fsxattr	attr;
	struct xflags	*p;
	unsigned int	i = 0;
	char		*c, *name = file->name;

	orflags = andflags = 0;
	recurse_all = recurse_dir = 0;
	while (++i < argc) {
		if (argv[i][0] == '-' && argv[i][1] == 'R') {
			recurse_all = 1;
		} else if (argv[i][0] == '-' && argv[i][1] == 'D') {
			recurse_dir = 1;
		} else if (argv[i][0] == '+') {
			for (c = &argv[i][1]; *c; c++) {
				for (p = xflags; p->flag; p++) {
					if (strncmp(p->shortname, c, 1) == 0) {
						orflags |= p->flag;
						break;
					}
				}
				if (!p->flag) {
					fprintf(stderr, _("%s: unknown flag\n"),
						progname);
					return 0;
				}
			}
		} else if (argv[i][0] == '-') {
			for (c = &argv[i][1]; *c; c++) {
				for (p = xflags; p->flag; p++) {
					if (strncmp(p->shortname, c, 1) == 0) {
						andflags |= p->flag;
						break;
					}
				}
				if (!p->flag) {
					fprintf(stderr, _("%s: unknown flag\n"),
						progname);
					return 0;
				}
			}
		} else {
			fprintf(stderr, _("%s: bad chattr command, not +/-X\n"),
				progname);
			return 0;
		}
	}

	if (recurse_all || recurse_dir) {
		nftw(name, chattr_callback,
			100, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
	} else if (xfsctl(name, file->fd, XFS_IOC_FSGETXATTR, &attr) < 0) {
		fprintf(stderr, _("%s: cannot get flags on %s: %s\n"),
			progname, name, strerror(errno));
	} else {
		attr.fsx_xflags |= orflags;
		attr.fsx_xflags &= ~andflags;
		if (xfsctl(name, file->fd, XFS_IOC_FSSETXATTR, &attr) < 0)
			fprintf(stderr, _("%s: cannot set flags on %s: %s\n"),
				progname, name, strerror(errno));
	}
	return 0;
}

void
attr_init(void)
{
	chattr_cmd.name = "chattr";
	chattr_cmd.cfunc = chattr_f;
	chattr_cmd.args = _("[-R|-D] [+/-"CHATTR_XFLAG_LIST"]");
	chattr_cmd.argmin = 1;
	chattr_cmd.argmax = -1;
	chattr_cmd.flags = CMD_NOMAP_OK;
	chattr_cmd.oneline =
		_("change extended inode flags on the currently open file");
	chattr_cmd.help = chattr_help;

	lsattr_cmd.name = "lsattr";
	lsattr_cmd.cfunc = lsattr_f;
	lsattr_cmd.args = _("[-R|-D|-a|-v]");
	lsattr_cmd.argmin = 0;
	lsattr_cmd.argmax = 1;
	lsattr_cmd.flags = CMD_NOMAP_OK;
	lsattr_cmd.oneline =
		_("list extended inode flags set on the currently open file");
	lsattr_cmd.help = lsattr_help;

	add_command(&chattr_cmd);
	add_command(&lsattr_cmd);
}
