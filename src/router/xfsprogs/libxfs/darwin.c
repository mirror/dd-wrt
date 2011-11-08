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

#include <sys/disk.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <xfs/libxfs.h>
#include <sys/sysctl.h>

int platform_has_uuid = 1;
extern char *progname;

int
platform_check_ismounted(char *name, char *block, struct stat64 *s, int verbose)
{
	return 0;
}

int
platform_check_iswritable(char *name, char *block, struct stat64 *s, int fatal)
{
	int	fd, writable;

	if ((fd = open(block, O_RDONLY, 0)) < 0) {
		fprintf(stderr, _("%s: "
			"error opening the device special file \"%s\": %s\n"),
			progname, block, strerror(errno));
		exit(1);
	}

	if (ioctl(fd, DKIOCISWRITABLE, &writable) < 0) {
		fprintf(stderr, _("%s: can't tell if \"%s\" is writable: %s\n"),
			progname, block, strerror(errno));
		exit(1);
	}
	close(fd);
	return (writable == 0);
}

int
platform_set_blocksize(int fd, char *path, dev_t device, int blocksize, int fatal)
{
	return fatal;
}

void
platform_flush_device(int fd, dev_t device)
{
	ioctl(fd, DKIOCSYNCHRONIZECACHE, NULL);
}

void
platform_findsizes(char *path, int fd, long long *sz, int *bsz)
{
	__uint64_t	size;
	struct stat64	st;

	if (fstat64(fd, &st) < 0) {
		fprintf(stderr,
			_("%s: cannot stat the device file \"%s\": %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}
	if ((st.st_mode & S_IFMT) == S_IFREG) {
		*sz = (long long)(st.st_size >> 9);
		*bsz = BBSIZE;
		return;
	}
	if (ioctl(fd, DKIOCGETBLOCKCOUNT, &size) < 0) {
		fprintf(stderr, _("%s: can't determine device size: %s\n"),
			progname, strerror(errno));
		exit(1);
	}
	*sz = (long long)size;
	*bsz = BBSIZE;
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
	return (sizeof(void *));
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

