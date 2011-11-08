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

static cmdinfo_t open_cmd;
static cmdinfo_t stat_cmd;
static cmdinfo_t close_cmd;
static cmdinfo_t setfl_cmd;
static cmdinfo_t statfs_cmd;
static cmdinfo_t chproj_cmd;
static cmdinfo_t lsproj_cmd;
static cmdinfo_t extsize_cmd;
static prid_t prid;
static long extsize;

off64_t
filesize(void)
{
	struct stat64	st;

	if (fstat64(file->fd, &st) < 0) {
		perror("fstat64");
		return -1;
	}
	return st.st_size;
}

static char *
filetype(mode_t mode)
{
	switch (mode & S_IFMT) {
	case S_IFSOCK:
		return _("socket");
	case S_IFDIR:
		return _("directory");
	case S_IFCHR:
		return _("char device");
	case S_IFBLK:
		return _("block device");
	case S_IFREG:
		return _("regular file");
	case S_IFLNK:
		return _("symbolic link");
	case S_IFIFO:
		return _("fifo");
	}
	return NULL;
}

static int
stat_f(
	int		argc,
	char		**argv)
{
	struct dioattr	dio;
	struct fsxattr	fsx, fsxa;
	struct stat64	st;
	int		verbose = (argc == 2 && !strcmp(argv[1], "-v"));

	printf(_("fd.path = \"%s\"\n"), file->name);
	printf(_("fd.flags = %s,%s,%s%s%s%s\n"),
		file->flags & IO_OSYNC ? _("sync") : _("non-sync"),
		file->flags & IO_DIRECT ? _("direct") : _("non-direct"),
		file->flags & IO_READONLY ? _("read-only") : _("read-write"),
		file->flags & IO_REALTIME ? _(",real-time") : "",
		file->flags & IO_APPEND ? _(",append-only") : "",
		file->flags & IO_NONBLOCK ? _(",non-block") : "");
	if (fstat64(file->fd, &st) < 0) {
		perror("fstat64");
	} else {
		printf(_("stat.ino = %lld\n"), (long long)st.st_ino);
		printf(_("stat.type = %s\n"), filetype(st.st_mode));
		printf(_("stat.size = %lld\n"), (long long)st.st_size);
		printf(_("stat.blocks = %lld\n"), (long long)st.st_blocks);
		if (verbose) {
			printf(_("stat.atime = %s"), ctime(&st.st_atime));
			printf(_("stat.mtime = %s"), ctime(&st.st_mtime));
			printf(_("stat.ctime = %s"), ctime(&st.st_ctime));
		}
	}
	if (file->flags & IO_FOREIGN)
		return 0;
	if ((xfsctl(file->name, file->fd, XFS_IOC_FSGETXATTR, &fsx)) < 0 ||
	    (xfsctl(file->name, file->fd, XFS_IOC_FSGETXATTRA, &fsxa)) < 0) {
		perror("XFS_IOC_FSGETXATTR");
	} else {
		printf(_("fsxattr.xflags = 0x%x "), fsx.fsx_xflags);
		printxattr(fsx.fsx_xflags, verbose, 0, file->name, 1, 1);
		printf(_("fsxattr.projid = %u\n"), fsx.fsx_projid);
		printf(_("fsxattr.extsize = %u\n"), fsx.fsx_extsize);
		printf(_("fsxattr.nextents = %u\n"), fsx.fsx_nextents);
		printf(_("fsxattr.naextents = %u\n"), fsxa.fsx_nextents);
	}
	if ((xfsctl(file->name, file->fd, XFS_IOC_DIOINFO, &dio)) < 0) {
		perror("XFS_IOC_DIOINFO");
	} else {
		printf(_("dioattr.mem = 0x%x\n"), dio.d_mem);
		printf(_("dioattr.miniosz = %u\n"), dio.d_miniosz);
		printf(_("dioattr.maxiosz = %u\n"), dio.d_maxiosz);
	}
	return 0;
}

int
openfile(
	char		*path,
	xfs_fsop_geom_t	*geom,
	int		flags,
	mode_t		mode)
{
	int		fd;
	int		oflags;

	oflags = flags & IO_READONLY ? O_RDONLY : O_RDWR;
	if (flags & IO_APPEND)
		oflags |= O_APPEND;
	if (flags & IO_CREAT)
		oflags |= O_CREAT;
	if (flags & IO_DIRECT)
		oflags |= O_DIRECT;
	if (flags & IO_OSYNC)
		oflags |= O_SYNC;
	if (flags & IO_TRUNC)
		oflags |= O_TRUNC;
	if (flags & IO_NONBLOCK)
		oflags |= O_NONBLOCK;

	fd = open(path, oflags, mode);
	if (fd < 0) {
		if ((errno == EISDIR) && (oflags & O_RDWR)) {
			/* make it as if we asked for O_RDONLY & try again */
			oflags &= ~O_RDWR;
			oflags |= O_RDONLY;
			flags |= IO_READONLY;
			fd = open(path, oflags, mode);
			if (fd < 0) {
				perror(path);
				return -1;
			}
		} else {
			perror(path);
			return -1;
		}
	}

	if (!geom)
		return fd;

	if (!platform_test_xfs_fd(fd)) {
		fprintf(stderr, _("%s: specified file "
			"[\"%s\"] is not on an XFS filesystem\n"),
			progname, path);
		close(fd);
		return -1;
	}

	if (xfsctl(path, fd, XFS_IOC_FSGEOMETRY, geom) < 0) {
		perror("XFS_IOC_FSGEOMETRY");
		close(fd);
		return -1;
	}

	if (!(flags & IO_READONLY) && (flags & IO_REALTIME)) {
		struct fsxattr	attr;

		if (xfsctl(path, fd, XFS_IOC_FSGETXATTR, &attr) < 0) {
			perror("XFS_IOC_FSGETXATTR");
			close(fd);
			return -1;
		}
		if (!(attr.fsx_xflags & XFS_XFLAG_REALTIME)) {
			attr.fsx_xflags |= XFS_XFLAG_REALTIME;
			if (xfsctl(path, fd, XFS_IOC_FSSETXATTR, &attr) < 0) {
				perror("XFS_IOC_FSSETXATTR");
				close(fd);
				return -1;
			}
		}
	}
	return fd;
}

int
addfile(
	char		*name,
	int		fd,
	xfs_fsop_geom_t	*geometry,
	int		flags)
{
	char		*filename;

	filename = strdup(name);
	if (!filename) {
		perror("strdup");
		close(fd);
		return -1;
	}

	/* Extend the table of currently open files */
	filetable = (fileio_t *)realloc(filetable,	/* growing */
					++filecount * sizeof(fileio_t));
	if (!filetable) {
		perror("realloc");
		filecount = 0;
		free(filename);
		close(fd);
		return -1;
	}

	/* Finally, make this the new active open file */
	file = &filetable[filecount - 1];
	file->fd = fd;
	file->flags = flags;
	file->name = filename;
	file->geom = *geometry;
	return 0;
}

static void
open_help(void)
{
	printf(_(
"\n"
" opens a new file in the requested mode\n"
"\n"
" Example:\n"
" 'open -cd /tmp/data' - creates/opens data file read-write for direct IO\n"
"\n"
" Opens a file for subsequent use by all of the other xfs_io commands.\n"
" With no arguments, open uses the stat command to show the current file.\n"
" -F -- foreign filesystem file, disallow XFS-specific commands\n"
" -a -- open with the O_APPEND flag (append-only mode)\n"
" -d -- open with O_DIRECT (non-buffered IO, note alignment constraints)\n"
" -f -- open with O_CREAT (create the file if it doesn't exist)\n"
" -m -- permissions to use in case a new file is created (default 0600)\n"
" -n -- open with O_NONBLOCK\n"
" -r -- open with O_RDONLY, the default is O_RDWR\n"
" -s -- open with O_SYNC\n"
" -t -- open with O_TRUNC (truncate the file to zero length if it exists)\n"
" -R -- mark the file as a realtime XFS file immediately after opening it\n"
" Note1: usually read/write direct IO requests must be blocksize aligned;\n"
"        some kernels, however, allow sectorsize alignment for direct IO.\n"
" Note2: the bmap for non-regular files can be obtained provided the file\n"
"        was opened correctly (in particular, must be opened read-only).\n"
"\n"));
}

static int
open_f(
	int		argc,
	char		**argv)
{
	int		c, fd, flags = 0;
	char		*sp;
	mode_t		mode = 0600;
	xfs_fsop_geom_t	geometry = { 0 };

	if (argc == 1) {
		if (file)
			return stat_f(argc, argv);
		fprintf(stderr, _("no files are open, try 'help open'\n"));
		return 0;
	}

	while ((c = getopt(argc, argv, "FRacdfm:nrstx")) != EOF) {
		switch (c) {
		case 'F':
			flags |= IO_FOREIGN;
			break;
		case 'a':
			flags |= IO_APPEND;
			break;
		case 'c':
		case 'f':
			flags |= IO_CREAT;
			break;
		case 'd':
			flags |= IO_DIRECT;
			break;
		case 'm':
			mode = strtoul(optarg, &sp, 0);
			if (!sp || sp == optarg) {
				printf(_("non-numeric mode -- %s\n"), optarg);
				return 0;
			}
			break;
		case 'n':
			flags |= IO_NONBLOCK;
			break;
		case 'r':
			flags |= IO_READONLY;
			break;
		case 's':
			flags |= IO_OSYNC;
			break;
		case 't':
			flags |= IO_TRUNC;
			break;
		case 'R':
		case 'x':	/* backwards compatibility */
			flags |= IO_REALTIME;
			break;
		default:
			return command_usage(&open_cmd);
		}
	}

	if (optind != argc - 1)
		return command_usage(&open_cmd);

	fd = openfile(argv[optind], flags & IO_FOREIGN ?
					NULL : &geometry, flags, mode);
	if (fd < 0)
		return 0;

	addfile(argv[optind], fd, &geometry, flags);
	return 0;
}

static int
close_f(
	int		argc,
	char		**argv)
{
	size_t		length;
	unsigned int	offset;

	if (close(file->fd) < 0) {
		perror("close");
		return 0;
	}
	free(file->name);

	/* Shuffle the file table entries down over the removed entry */
	offset = file - &filetable[0];
	length = filecount * sizeof(fileio_t);
	length -= (offset + 1) * sizeof(fileio_t);
	if (length)
		memmove(file, file + 1, length);

	/* Resize the memory allocated for the table, possibly freeing */
	if (--filecount) {
		filetable = (fileio_t *)realloc(filetable,	/* shrinking */
					filecount * sizeof(fileio_t));
		if (offset == filecount)
			offset--;
		file = filetable + offset;
	} else {
		free(filetable);
		file = filetable = NULL;
	}
	filelist_f();
	return 0;
}

static void
lsproj_help(void)
{
	printf(_(
"\n"
" displays the project identifier associated with the current path\n"
"\n"
" Options:\n"
" -R -- recursively descend (useful when current path is a directory)\n"
" -D -- recursively descend, but only list projects on directories\n"
"\n"));
}

static int
lsproj_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	prid_t			projid;
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	if ((fd = open(path, O_RDONLY)) == -1) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		if (getprojid(path, fd, &projid) == 0)
			printf("[%u] %s\n", (unsigned int)projid, path);
		close(fd);
	}
	return 0;
}

static int
lsproj_f(
	int		argc,
	char		**argv)
{
	prid_t		projid;
	int		c;

	recurse_all = recurse_dir = 0;
	while ((c = getopt(argc, argv, "DR")) != EOF) {
		switch (c) {
		case 'D':
			recurse_all = 0;
			recurse_dir = 1;
			break;
		case 'R':
			recurse_all = 1;
			recurse_dir = 0;
			break;
		default:
			return command_usage(&lsproj_cmd);
		}
	}

	if (argc != optind)
		return command_usage(&lsproj_cmd);

	if (recurse_all || recurse_dir)
		nftw(file->name, lsproj_callback,
			100, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
	else if (getprojid(file->name, file->fd, &projid) < 0)
		perror("getprojid");
	else
		printf(_("projid = %u\n"), (unsigned int)projid);
	return 0;
}

static void
chproj_help(void)
{
	printf(_(
"\n"
" modifies the project identifier associated with the current path\n"
"\n"
" -R -- recursively descend (useful when current path is a directory)\n"
" -D -- recursively descend, only modifying projects on directories\n"
"\n"));
}

static int
chproj_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	if ((fd = open(path, O_RDONLY)) == -1) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		if (setprojid(path, fd, prid) < 0)
			perror("setprojid");
		close(fd);
	}
	return 0;
}

static int
chproj_f(
	int		argc,
	char		**argv)
{
	int		c;

	recurse_all = recurse_dir = 0;
	while ((c = getopt(argc, argv, "DR")) != EOF) {
		switch (c) {
		case 'D':
			recurse_all = 0;
			recurse_dir = 1;
			break;
		case 'R':
			recurse_all = 1;
			recurse_dir = 0;
			break;
		default:
			return command_usage(&chproj_cmd);
		}
	}

	if (argc != optind + 1)
		return command_usage(&chproj_cmd);

	prid = prid_from_string(argv[optind]);
	if (prid == -1) {
		printf(_("invalid project ID -- %s\n"), argv[optind]);
		return 0;
	}

	if (recurse_all || recurse_dir)
		nftw(file->name, chproj_callback,
			100, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
	else if (setprojid(file->name, file->fd, prid) < 0)
		perror("setprojid");
	return 0;
}

static void
extsize_help(void)
{
	printf(_(
"\n"
" report or modify preferred extent size (in bytes) for the current path\n"
"\n"
" -R -- recursively descend (useful when current path is a directory)\n"
" -D -- recursively descend, only modifying extsize on directories\n"
"\n"));
}

static int
get_extsize(const char *path, int fd)
{
	struct fsxattr	fsx;

	if ((xfsctl(path, fd, XFS_IOC_FSGETXATTR, &fsx)) < 0) {
		printf("%s: XFS_IOC_FSGETXATTR %s: %s\n",
			progname, path, strerror(errno));
		return 0;
	}
	printf("[%u] %s\n", fsx.fsx_extsize, path);
	return 0;
}

static int
set_extsize(const char *path, int fd, long extsz)
{
	struct fsxattr	fsx;
	struct stat64	stat;

	if (fstat64(fd, &stat) < 0) {
		perror("fstat64");
		return 0;
	}
	if ((xfsctl(path, fd, XFS_IOC_FSGETXATTR, &fsx)) < 0) {
		printf("%s: XFS_IOC_FSGETXATTR %s: %s\n",
			progname, path, strerror(errno));
		return 0;
	}

	if (S_ISREG(stat.st_mode)) {
		fsx.fsx_xflags |= XFS_XFLAG_EXTSIZE;
	} else if (S_ISDIR(stat.st_mode)) {
		fsx.fsx_xflags |= XFS_XFLAG_EXTSZINHERIT;
	} else {
		printf(_("invalid target file type - file %s\n"), path);
		return 0;
	}
	fsx.fsx_extsize = extsz;

	if ((xfsctl(path, fd, XFS_IOC_FSSETXATTR, &fsx)) < 0) {
		printf("%s: XFS_IOC_FSSETXATTR %s: %s\n",
			progname, path, strerror(errno));
		return 0;
	}

	return 0;
}

static int
get_extsize_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	if ((fd = open(path, O_RDONLY)) == -1) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		get_extsize(path, fd);
		close(fd);
	}
	return 0;
}

static int
set_extsize_callback(
	const char		*path,
	const struct stat	*stat,
	int			status,
	struct FTW		*data)
{
	int			fd;

	if (recurse_dir && !S_ISDIR(stat->st_mode))
		return 0;

	if ((fd = open(path, O_RDONLY)) == -1) {
		fprintf(stderr, _("%s: cannot open %s: %s\n"),
			progname, path, strerror(errno));
	} else {
		set_extsize(path, fd, extsize);
		close(fd);
	}
	return 0;
}

static int
extsize_f(
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
			recurse_all = 0;
			recurse_dir = 1;
			break;
		case 'R':
			recurse_all = 1;
			recurse_dir = 0;
			break;
		default:
			return command_usage(&extsize_cmd);
		}
	}

	if (optind < argc) {
		extsize = (long)cvtnum(blocksize, sectsize, argv[optind]);
		if (extsize < 0) {
			printf(_("non-numeric extsize argument -- %s\n"),
				argv[optind]);
			return 0;
		}
	} else {
		extsize = -1;
	}

	if (recurse_all || recurse_dir)
		nftw(file->name, (extsize >= 0) ?
			set_extsize_callback : get_extsize_callback,
			100, FTW_PHYS | FTW_MOUNT | FTW_DEPTH);
	else if (extsize >= 0)
		set_extsize(file->name, file->fd, extsize);
	else
		get_extsize(file->name, file->fd);
	return 0;
}

static int
setfl_f(
	int			argc,
	char			**argv)
{
	int			c, flags;

	flags = fcntl(file->fd, F_GETFL, 0);
	if (flags < 0) {
		perror("fcntl(F_GETFL)");
		return 0;
	}

	while ((c = getopt(argc, argv, "ad")) != EOF) {
		switch (c) {
		case 'a':
			if (flags & O_APPEND)
				flags |= O_APPEND;
			else
				flags &= ~O_APPEND;
			break;
		case 'd':
			if (flags & O_DIRECT)
				flags |= O_DIRECT;
			else
				flags &= ~O_DIRECT;
			break;
		default:
			printf(_("invalid setfl argument -- '%c'\n"), c);
			return 0;
		}
	}

	if (fcntl(file->fd, F_SETFL, flags)  < 0)
		perror("fcntl(F_SETFL)");

	return 0;
}

static int
statfs_f(
	int			argc,
	char			**argv)
{
	struct xfs_fsop_counts	fscounts;
	struct xfs_fsop_geom	fsgeo;
	struct statfs		st;

	printf(_("fd.path = \"%s\"\n"), file->name);
	if (platform_fstatfs(file->fd, &st) < 0) {
		perror("fstatfs");
	} else {
		printf(_("statfs.f_bsize = %lld\n"), (long long) st.f_bsize);
		printf(_("statfs.f_blocks = %lld\n"), (long long) st.f_blocks);
#if defined(__sgi__)
		printf(_("statfs.f_frsize = %lld\n"), (long long) st.f_frsize);
#else
		printf(_("statfs.f_bavail = %lld\n"), (long long) st.f_bavail);
#endif
		printf(_("statfs.f_files = %lld\n"), (long long) st.f_files);
		printf(_("statfs.f_ffree = %lld\n"), (long long) st.f_ffree);
	}
	if (file->flags & IO_FOREIGN)
		return 0;
	if ((xfsctl(file->name, file->fd, XFS_IOC_FSGEOMETRY_V1, &fsgeo)) < 0) {
		perror("XFS_IOC_FSGEOMETRY_V1");
	} else {
		printf(_("geom.bsize = %u\n"), fsgeo.blocksize);
		printf(_("geom.agcount = %u\n"), fsgeo.agcount);
		printf(_("geom.agblocks = %u\n"), fsgeo.agblocks);
		printf(_("geom.datablocks = %llu\n"),
			(unsigned long long) fsgeo.datablocks);
		printf(_("geom.rtblocks = %llu\n"),
			(unsigned long long) fsgeo.rtblocks);
		printf(_("geom.rtextents = %llu\n"),
			(unsigned long long) fsgeo.rtextents);
		printf(_("geom.rtextsize = %u\n"), fsgeo.rtextsize);
		printf(_("geom.sunit = %u\n"), fsgeo.sunit);
		printf(_("geom.swidth = %u\n"), fsgeo.swidth);
	}
	if ((xfsctl(file->name, file->fd, XFS_IOC_FSCOUNTS, &fscounts)) < 0) {
		perror("XFS_IOC_FSCOUNTS");
	} else {
		printf(_("counts.freedata = %llu\n"),
			(unsigned long long) fscounts.freedata);
		printf(_("counts.freertx = %llu\n"),
			(unsigned long long) fscounts.freertx);
		printf(_("counts.freeino = %llu\n"),
			(unsigned long long) fscounts.freeino);
		printf(_("counts.allocino = %llu\n"),
			(unsigned long long) fscounts.allocino);
	}
	return 0;
}

void
open_init(void)
{
	open_cmd.name = "open";
	open_cmd.altname = "o";
	open_cmd.cfunc = open_f;
	open_cmd.argmin = 0;
	open_cmd.argmax = -1;
	open_cmd.flags = CMD_NOMAP_OK | CMD_NOFILE_OK | CMD_FOREIGN_OK;
	open_cmd.args = _("[-acdrstx] [path]");
	open_cmd.oneline = _("open the file specified by path");
	open_cmd.help = open_help;

	stat_cmd.name = "stat";
	stat_cmd.cfunc = stat_f;
	stat_cmd.argmin = 0;
	stat_cmd.argmax = 1;
	stat_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	stat_cmd.args = _("[-v]");
	stat_cmd.oneline = _("statistics on the currently open file");

	close_cmd.name = "close";
	close_cmd.altname = "c";
	close_cmd.cfunc = close_f;
	close_cmd.argmin = 0;
	close_cmd.argmax = 0;
	close_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	close_cmd.oneline = _("close the current open file");

	setfl_cmd.name = "setfl";
	setfl_cmd.cfunc = setfl_f;
	setfl_cmd.args = _("[-adx]");
	setfl_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	setfl_cmd.oneline =
		_("set/clear append/direct flags on the open file");

	statfs_cmd.name = "statfs";
	statfs_cmd.cfunc = statfs_f;
	statfs_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	statfs_cmd.oneline =
		_("statistics on the filesystem of the currently open file");

	chproj_cmd.name = "chproj";
	chproj_cmd.cfunc = chproj_f;
	chproj_cmd.args = _("[-D | -R] projid");
	chproj_cmd.argmin = 1;
	chproj_cmd.argmax = -1;
	chproj_cmd.flags = CMD_NOMAP_OK;
	chproj_cmd.oneline =
		_("change project identifier on the currently open file");
	chproj_cmd.help = chproj_help;

	lsproj_cmd.name = "lsproj";
	lsproj_cmd.cfunc = lsproj_f;
	lsproj_cmd.args = _("[-D | -R]");
	lsproj_cmd.argmin = 0;
	lsproj_cmd.argmax = -1;
	lsproj_cmd.flags = CMD_NOMAP_OK;
	lsproj_cmd.oneline =
		_("list project identifier set on the currently open file");
	lsproj_cmd.help = lsproj_help;

	extsize_cmd.name = "extsize";
	extsize_cmd.cfunc = extsize_f;
	extsize_cmd.args = _("[-D | -R] [extsize]");
	extsize_cmd.argmin = 0;
	extsize_cmd.argmax = -1;
	extsize_cmd.flags = CMD_NOMAP_OK;
	extsize_cmd.oneline =
		_("get/set preferred extent size (in bytes) for the open file");
	extsize_cmd.help = extsize_help;

	add_command(&open_cmd);
	add_command(&stat_cmd);
	add_command(&close_cmd);
	add_command(&setfl_cmd);
	add_command(&statfs_cmd);
	add_command(&chproj_cmd);
	add_command(&lsproj_cmd);
	add_command(&extsize_cmd);
}
