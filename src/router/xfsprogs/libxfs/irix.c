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

#include <xfs/libxfs.h>
#include <diskinfo.h>
#include <sys/sysmp.h>

int platform_has_uuid = 0;
extern char *progname;
extern __int64_t findsize(char *);

int
platform_check_ismounted(char *name, char *block, struct stat64 *s, int verbose)
{
	return 0;
}

int
platform_check_iswritable(char *name, char *block, struct stat64 *s, int fatal)
{
	return 1;
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
	struct stat64		st;

	if (fstat64(fd, &st) < 0) {
		fprintf(stderr,
			_("%s: cannot stat the device file \"%s\": %s\n"),
			progname, path, strerror(errno));
		exit(1);
	}
	if ((st.st_mode & S_IFMT) == S_IFREG) {
		*sz = (long long)(st.st_size >> 9);
	} else {
		*sz = findsize(path);
	}
	*bsz = BBSIZE;
}

char *
platform_findrawpath(char *path)
{
	return findrawpath(path);
}

char *
platform_findblockpath(char *path)
{
	return findblockpath(path);
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
	return sysmp(MP_NPROCS);
}

unsigned long
platform_physmem(void)
{
	struct rminfo ri;

	if (sysmp(MP_SAGET, MPSA_RMINFO, &ri, sizeof(ri)) < 0)
		fprintf(stderr, _("%s: can't determine memory size\n"),
			progname);
		exit(1);
	}
	return (ri.physmem >> 10) * getpagesize();	/* kilobytes */
}