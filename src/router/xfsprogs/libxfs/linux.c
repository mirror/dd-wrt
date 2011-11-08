/*
 * Copyright (c) 2000-2003,2005 Silicon Graphics, Inc.
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

#define ustat __kernel_ustat
#include <xfs/libxfs.h>
#include <mntent.h>
#include <sys/stat.h>
#undef ustat
#ifndef __UCLIBC__
#include <sys/ustat.h>
#endif
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

int platform_has_uuid = 1;
extern char *progname;
static int max_block_alignment;

#ifndef BLKGETSIZE64
# define BLKGETSIZE64	_IOR(0x12,114,size_t)
#endif
#ifndef BLKBSZSET
# define BLKBSZSET	_IOW(0x12,113,size_t)
#endif
#ifndef BLKSSZGET
# define BLKSSZGET	_IO(0x12,104)
#endif

#ifndef RAMDISK_MAJOR
#define RAMDISK_MAJOR	1	/* ramdisk major number */
#endif

#define PROC_MOUNTED	"/proc/mounts"

int
platform_check_ismounted(char *name, char *block, struct stat64 *s, int verbose)
{
#ifndef __UCLIBC__
	/* Pad ust; pre-2.6.28 linux copies out too much in 32bit compat mode */
	struct ustat	ust[2];
	struct stat64	st;

	if (!s) {
		if (stat64(block, &st) < 0)
			return 0;
		if ((st.st_mode & S_IFMT) != S_IFBLK)
			return 0;
		s = &st;
	}

	if (ustat(s->st_rdev, ust) >= 0) {
		if (verbose)
			fprintf(stderr,
				_("%s: %s contains a mounted filesystem\n"),
				progname, name);
		return 1;
	}
#endif
	return 0;
}

int
platform_check_iswritable(char *name, char *block, struct stat64 *s, int fatal)
{
	int		sts = 0;
	FILE		*f;
	struct stat64	mst;
	struct mntent	*mnt;
	char		mounts[MAXPATHLEN];

	strcpy(mounts, (!access(PROC_MOUNTED, R_OK)) ? PROC_MOUNTED : MOUNTED);
	if ((f = setmntent(mounts, "r")) == NULL) {
		fprintf(stderr, _("%s: %s contains a possibly writable, "
				"mounted filesystem\n"), progname, name);
			return fatal;
	}
	while ((mnt = getmntent(f)) != NULL) {
		if (stat64(mnt->mnt_fsname, &mst) < 0)
			continue;
		if ((mst.st_mode & S_IFMT) != S_IFBLK)
			continue;
		if (mst.st_rdev == s->st_rdev
		    && hasmntopt(mnt, MNTOPT_RO) != NULL)
			break;
	}
	if (mnt == NULL) {
		fprintf(stderr, _("%s: %s contains a mounted and writable "
				"filesystem\n"), progname, name);
		sts = fatal;
	}
	endmntent(f);
	return sts;
}

int
platform_set_blocksize(int fd, char *path, dev_t device, int blocksize, int fatal)
{
	int error = 0;

	if (major(device) != RAMDISK_MAJOR) {
		if ((error = ioctl(fd, BLKBSZSET, &blocksize)) < 0) {
			fprintf(stderr, _("%s: %s - cannot set blocksize "
					"%d on block device %s: %s\n"),
				progname, fatal ? "error": "warning",
				blocksize, path, strerror(errno));
		}
	}
	return error;
}

void
platform_flush_device(int fd, dev_t device)
{
	if (major(device) != RAMDISK_MAJOR)
		ioctl(fd, BLKFLSBUF, 0);
}

void
platform_findsizes(char *path, int fd, long long *sz, int *bsz)
{
	struct stat64	st;
	__uint64_t	size;
	int		error;

	if (fstat64(fd, &st) < 0) {
		fprintf(stderr, _("%s: "
			"cannot stat the device file \"%s\": %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}
	if ((st.st_mode & S_IFMT) == S_IFREG) {
		*sz = (long long)(st.st_size >> 9);
		*bsz = BBSIZE;
		if (BBSIZE > max_block_alignment)
			max_block_alignment = BBSIZE;
		return;
	}

	error = ioctl(fd, BLKGETSIZE64, &size);
	if (error >= 0) {
		/* BLKGETSIZE64 returns size in bytes not 512-byte blocks */
		*sz = (long long)(size >> 9);
	} else {
		/* If BLKGETSIZE64 fails, try BLKGETSIZE */
		unsigned long tmpsize;

		error = ioctl(fd, BLKGETSIZE, &tmpsize);
		if (error < 0) {
			fprintf(stderr, _("%s: can't determine device size\n"),
				progname);
			exit(1);
		}
		*sz = (long long)tmpsize;
	}

	if (ioctl(fd, BLKSSZGET, bsz) < 0) {
		fprintf(stderr, _("%s: warning - cannot get sector size "
				"from block device %s: %s\n"),
			progname, path, strerror(errno));
		*bsz = BBSIZE;
	}
	if (*bsz > max_block_alignment)
		max_block_alignment = *bsz;
}

char *
platform_findrawpath(char *path)
{
	return path;
}

char *
platform_findblockpath(char *path)
{
	return path;
}

int
platform_direct_blockdev(void)
{
	return 1;
}

int
platform_align_blockdev(void)
{
	if (!max_block_alignment)
		return getpagesize();
	return max_block_alignment;
}

int
platform_nproc(void)
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

unsigned long
platform_physmem(void)
{
	struct sysinfo  si;

	if (sysinfo(&si) < 0) {
		fprintf(stderr, _("%s: can't determine memory size\n"),
			progname);
		exit(1);
	}
	return (si.totalram >> 10) * si.mem_unit;	/* kilobytes */
}
