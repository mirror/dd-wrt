/*
 * Copyright (c) 2003-2005 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * Copyright (C) 2015, 2017 Red Hat, Inc.
 * Portions of statx support written by David Howells (dhowells@redhat.com)
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

#include "command.h"
#include "input.h"
#include "init.h"
#include "io.h"
#include "statx.h"
#include "libxfs.h"

#include <fcntl.h>

static cmdinfo_t stat_cmd;
static cmdinfo_t statfs_cmd;
static cmdinfo_t statx_cmd;

off64_t
filesize(void)
{
	struct stat	st;

	if (fstat(file->fd, &st) < 0) {
		perror("fstat");
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
dump_raw_stat(struct stat *st)
{
	printf("stat.blksize = %lu\n", (unsigned long)st->st_blksize);
	printf("stat.nlink = %lu\n", (unsigned long)st->st_nlink);
	printf("stat.uid = %u\n", st->st_uid);
	printf("stat.gid = %u\n", st->st_gid);
	printf("stat.mode: 0%o\n", st->st_mode);
	printf("stat.ino = %llu\n", (unsigned long long)st->st_ino);
	printf("stat.size = %lld\n", (long long)st->st_size);
	printf("stat.blocks = %lld\n", (long long)st->st_blocks);
	printf("stat.atime.tv_sec = %ld\n", st->st_atim.tv_sec);
	printf("stat.atime.tv_nsec = %ld\n", st->st_atim.tv_nsec);
	printf("stat.ctime.tv_sec = %ld\n", st->st_ctim.tv_sec);
	printf("stat.ctime.tv_nsec = %ld\n", st->st_ctim.tv_nsec);
	printf("stat.mtime.tv_sec = %ld\n", st->st_mtim.tv_sec);
	printf("stat.mtime.tv_nsec = %ld\n", st->st_mtim.tv_nsec);
	printf("stat.rdev_major = %u\n", major(st->st_rdev));
	printf("stat.rdev_minor = %u\n", minor(st->st_rdev));
	printf("stat.dev_major = %u\n", major(st->st_dev));
	printf("stat.dev_minor = %u\n", minor(st->st_dev));
	return 0;
}

void print_file_info(void)
{
	printf(_("fd.path = \"%s\"\n"), file->name);
	printf(_("fd.flags = %s,%s,%s%s%s%s%s\n"),
		file->flags & IO_OSYNC ? _("sync") : _("non-sync"),
		file->flags & IO_DIRECT ? _("direct") : _("non-direct"),
		file->flags & IO_READONLY ? _("read-only") : _("read-write"),
		file->flags & IO_REALTIME ? _(",real-time") : "",
		file->flags & IO_APPEND ? _(",append-only") : "",
		file->flags & IO_NONBLOCK ? _(",non-block") : "",
		file->flags & IO_TMPFILE ? _(",tmpfile") : "");
}

void print_xfs_info(int verbose)
{
	struct dioattr	dio;
	struct fsxattr	fsx, fsxa;

	if ((xfsctl(file->name, file->fd, FS_IOC_FSGETXATTR, &fsx)) < 0 ||
	    (xfsctl(file->name, file->fd, XFS_IOC_FSGETXATTRA, &fsxa)) < 0) {
		perror("FS_IOC_FSGETXATTR");
	} else {
		printf(_("fsxattr.xflags = 0x%x "), fsx.fsx_xflags);
		printxattr(fsx.fsx_xflags, verbose, 0, file->name, 1, 1);
		printf(_("fsxattr.projid = %u\n"), fsx.fsx_projid);
		printf(_("fsxattr.extsize = %u\n"), fsx.fsx_extsize);
		printf(_("fsxattr.cowextsize = %u\n"), fsx.fsx_cowextsize);
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
}

int
stat_f(
	int		argc,
	char		**argv)
{
	struct stat	st;
	int		c, verbose = 0, raw = 0;

	while ((c = getopt(argc, argv, "rv")) != EOF) {
		switch (c) {
		case 'r':
			raw = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			return command_usage(&stat_cmd);
		}
	}

	if (raw && verbose)
		return command_usage(&stat_cmd);

	if (fstat(file->fd, &st) < 0) {
		perror("fstat");
		return 0;
	}

	if (raw)
		return dump_raw_stat(&st);

	print_file_info();

	printf(_("stat.ino = %lld\n"), (long long)st.st_ino);
	printf(_("stat.type = %s\n"), filetype(st.st_mode));
	printf(_("stat.size = %lld\n"), (long long)st.st_size);
	printf(_("stat.blocks = %lld\n"), (long long)st.st_blocks);
	if (verbose) {
		printf(_("stat.atime = %s"), ctime(&st.st_atime));
		printf(_("stat.mtime = %s"), ctime(&st.st_mtime));
		printf(_("stat.ctime = %s"), ctime(&st.st_ctime));
	}

	if (file->flags & IO_FOREIGN)
		return 0;

	print_xfs_info(verbose);

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
		printf(_("statfs.f_bavail = %lld\n"), (long long) st.f_bavail);
		printf(_("statfs.f_files = %lld\n"), (long long) st.f_files);
		printf(_("statfs.f_ffree = %lld\n"), (long long) st.f_ffree);
#ifdef HAVE_STATFS_FLAGS
		printf(_("statfs.f_flags = 0x%llx\n"), (long long) st.f_flags);
#endif
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

static ssize_t
_statx(
	int		dfd,
	const char	*filename,
	unsigned int	flags,
	unsigned int	mask,
	struct statx	*buffer)
{
#ifdef __NR_statx
	return syscall(__NR_statx, dfd, filename, flags, mask, buffer);
#else
	errno = ENOSYS;
	return -1;
#endif
}

static void
statx_help(void)
{
        printf(_(
"\n"
" Display extended file status.\n"
"\n"
" Options:\n"
" -v -- More verbose output\n"
" -r -- Print raw statx structure fields\n"
" -m mask -- Specify the field mask for the statx call\n"
"            (can also be 'basic' or 'all'; default STATX_ALL)\n"
" -D -- Don't sync attributes with the server\n"
" -F -- Force the attributes to be sync'd with the server\n"
"\n"));
}

/* statx helper */
static int
dump_raw_statx(struct statx *stx)
{
	printf("stat.mask = 0x%x\n", stx->stx_mask);
	printf("stat.blksize = %u\n", stx->stx_blksize);
	printf("stat.attributes = 0x%llx\n", (unsigned long long)stx->stx_attributes);
	printf("stat.nlink = %u\n", stx->stx_nlink);
	printf("stat.uid = %u\n", stx->stx_uid);
	printf("stat.gid = %u\n", stx->stx_gid);
	printf("stat.mode: 0%o\n", stx->stx_mode);
	printf("stat.ino = %llu\n", (unsigned long long)stx->stx_ino);
	printf("stat.size = %llu\n", (unsigned long long)stx->stx_size);
	printf("stat.blocks = %llu\n", (unsigned long long)stx->stx_blocks);
	printf("stat.atime.tv_sec = %lld\n", (long long)stx->stx_atime.tv_sec);
	printf("stat.atime.tv_nsec = %d\n", stx->stx_atime.tv_nsec);
	printf("stat.btime.tv_sec = %lld\n", (long long)stx->stx_btime.tv_sec);
	printf("stat.btime.tv_nsec = %d\n", stx->stx_btime.tv_nsec);
	printf("stat.ctime.tv_sec = %lld\n", (long long)stx->stx_ctime.tv_sec);
	printf("stat.ctime.tv_nsec = %d\n", stx->stx_ctime.tv_nsec);
	printf("stat.mtime.tv_sec = %lld\n", (long long)stx->stx_mtime.tv_sec);
	printf("stat.mtime.tv_nsec = %d\n", stx->stx_mtime.tv_nsec);
	printf("stat.rdev_major = %u\n", stx->stx_rdev_major);
	printf("stat.rdev_minor = %u\n", stx->stx_rdev_minor);
	printf("stat.dev_major = %u\n", stx->stx_dev_major);
	printf("stat.dev_minor = %u\n", stx->stx_dev_minor);
	return 0;
}

/*
 * options:
 * 	- input flags - query type
 * 	- output style for flags (and all else?) (chars vs. hex?)
 * 	- output - mask out incidental flag or not?
 */
int
statx_f(
	int		argc,
	char		**argv)
{
	int		c, verbose = 0, raw = 0;
	char		*p;
	struct statx	stx;
	int		atflag = 0;
	unsigned int	mask = STATX_ALL;

	while ((c = getopt(argc, argv, "m:rvFD")) != EOF) {
		switch (c) {
		case 'm':
			if (strcmp(optarg, "basic") == 0)
				mask = STATX_BASIC_STATS;
			else if (strcmp(optarg, "all") == 0)
				mask = STATX_ALL;
			else {
				mask = strtoul(optarg, &p, 0);
				if (!p || p == optarg) {
					printf(
				_("non-numeric mask -- %s\n"), optarg);
					return 0;
				}
			}
			break;
		case 'r':
			raw = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'F':
			atflag &= ~AT_STATX_SYNC_TYPE;
			atflag |= AT_STATX_FORCE_SYNC;
			break;
		case 'D':
			atflag &= ~AT_STATX_SYNC_TYPE;
			atflag |= AT_STATX_DONT_SYNC;
			break;
		default:
			return command_usage(&statx_cmd);
		}
	}

	if (raw && verbose)
		return command_usage(&statx_cmd);

	memset(&stx, 0xbf, sizeof(stx));
	if (_statx(file->fd, "", atflag | AT_EMPTY_PATH, mask, &stx) < 0) {
		perror("statx");
		return 0;
	}

	if (raw)
		return dump_raw_statx(&stx);

	print_file_info();

	printf(_("stat.ino = %lld\n"), (long long)stx.stx_ino);
	printf(_("stat.type = %s\n"), filetype(stx.stx_mode));
	printf(_("stat.size = %lld\n"), (long long)stx.stx_size);
	printf(_("stat.blocks = %lld\n"), (long long)stx.stx_blocks);
	if (verbose) {
		printf(_("stat.atime = %s"), ctime((time_t *)&stx.stx_atime.tv_sec));
		printf(_("stat.mtime = %s"), ctime((time_t *)&stx.stx_mtime.tv_sec));
		printf(_("stat.ctime = %s"), ctime((time_t *)&stx.stx_ctime.tv_sec));
		if (stx.stx_mask & STATX_BTIME)
			printf(_("stat.btime = %s"),
				ctime((time_t *)&stx.stx_btime.tv_sec));
	}

	if (file->flags & IO_FOREIGN)
		return 0;

	print_xfs_info(verbose);

	return 0;
}

void
stat_init(void)
{
	stat_cmd.name = "stat";
	stat_cmd.cfunc = stat_f;
	stat_cmd.argmin = 0;
	stat_cmd.argmax = 1;
	stat_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	stat_cmd.args = _("[-v|-r]");
	stat_cmd.oneline = _("statistics on the currently open file");

	statx_cmd.name = "statx";
	statx_cmd.cfunc = statx_f;
	statx_cmd.argmin = 0;
	statx_cmd.argmax = -1;
	statx_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	statx_cmd.args = _("[-v|-r][-m basic | -m all | -m <mask>][-FD]");
	statx_cmd.oneline = _("extended statistics on the currently open file");
	statx_cmd.help = statx_help;

	statfs_cmd.name = "statfs";
	statfs_cmd.cfunc = statfs_f;
	statfs_cmd.flags = CMD_NOMAP_OK | CMD_FOREIGN_OK;
	statfs_cmd.oneline =
		_("statistics on the filesystem of the currently open file");

	add_command(&stat_cmd);
	add_command(&statx_cmd);
	add_command(&statfs_cmd);
}
