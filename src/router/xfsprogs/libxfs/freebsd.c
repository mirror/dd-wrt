/*
 * Copyright (c) 2003,2005 Silicon Graphics, Inc.
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

#include "libxfs.h"
#include <sys/stat.h>
#include <sys/disk.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>

int platform_has_uuid = 1;
extern char *progname;

int
platform_check_ismounted(char *name, char *block, struct stat *s, int verbose)
{
	struct stat	st;
        int cnt, i;
        struct statfs *fsinfo;

	if (!s) {
		if (stat(block, &st) < 0)
			return 0;
		s = &st;
	}

	/* Remember, FreeBSD can now mount char devices! -- adrian */
	if (((st.st_mode & S_IFMT) != S_IFBLK) &&
	    ((st.st_mode & S_IFMT) != S_IFCHR))
		return 0;

	if ((cnt = getmntinfo(&fsinfo, MNT_NOWAIT)) == 0) {
		fprintf(stderr,
		    _("%s: %s possibly contains a mounted filesystem\n"),
		    progname, name);
		return 1;
	}

        for (i = 0; i < cnt; i++) {
                if (strcmp (name, fsinfo[i].f_mntfromname) != 0)
			continue;

		if (verbose)
			fprintf(stderr,
			    _("%s: %s contains a mounted filesystem\n"),
			    progname, name);
		break;
	}

        return i < cnt;
}

int
platform_check_iswritable(char *name, char *block, struct stat *s)
{
        int cnt, i;
        struct statfs *fsinfo;

        if ((cnt = getmntinfo(&fsinfo, MNT_NOWAIT)) == 0) {
		fprintf(stderr, _("%s: %s contains a possibly writable, "
				"mounted filesystem\n"), progname, name);
			return 1;
	}

        for (i = 0; i < cnt; i++) {
                if (strcmp (name, fsinfo[i].f_mntfromname) != 0)
			continue;

		if (fsinfo[i].f_flags &= MNT_RDONLY)
			break;
	}

        if (i == cnt) {
		fprintf(stderr, _("%s: %s contains a mounted and writable "
				"filesystem\n"), progname, name);
		return 1;
	}
	return 0;
}

int
platform_set_blocksize(int fd, char *path, dev_t device, int blocksize, int fatal)
{
	return fatal;
}

void
platform_flush_device(int fd, dev_t device)
{
	return;
}

void
platform_findsizes(char *path, int fd, long long *sz, int *bsz)
{
	struct stat	st;
	int64_t		size;
	uint		ssize;

	if (fstat(fd, &st) < 0) {
		fprintf(stderr, _("%s: "
			"cannot stat the device file \"%s\": %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}

	if ((st.st_mode & S_IFMT) == S_IFREG) {
		*sz = (long long)(st.st_size >> 9);
		*bsz = 512;
		return;
	}

	if ((st.st_mode & S_IFMT) != S_IFCHR) {
		fprintf(stderr, _("%s: Not a device or file: \"%s\"\n"),
			progname, path);
		exit(1);
	}

	if (ioctl(fd, DIOCGMEDIASIZE, &size) != 0) {
		fprintf(stderr, _("%s: DIOCGMEDIASIZE failed on \"%s\": %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}

	if (ioctl(fd, DIOCGSECTORSIZE, &ssize) != 0) {
		fprintf(stderr, _("%s: "
			"DIOCGSECTORSIZE failed on \"%s\": %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}

	*sz = (long long) (size / ssize);
	*bsz = (int)ssize;
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
	return 0;
}

int
platform_align_blockdev(void)
{
	return sizeof(void *);
}

int
platform_nproc(void)
{
	int		ncpu;
	size_t		len = sizeof(ncpu);
	static int	mib[2] = {CTL_HW, HW_NCPU};

	if (sysctl(mib, 2, &ncpu, &len, NULL, 0) < 0)
		ncpu = 1;

	return ncpu;
}

unsigned long
platform_physmem(void)
{
	unsigned long	physmem;
	size_t		len = sizeof(physmem);
	static int	mib[2] = {CTL_HW, HW_PHYSMEM};

	if (sysctl(mib, 2, &physmem, &len, NULL, 0) < 0) {
		fprintf(stderr, _("%s: can't determine memory size\n"),
			progname);
		exit(1);
	}
	return physmem >> 10;
}
