/*
 * Copyright (c) 2004-2006 Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef __XFS_KFREEBSD_H__
#define __XFS_KFREEBSD_H__

#include <uuid/uuid.h>
#include <sys/vfs.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>
#include <malloc.h>
#include <getopt.h>
#include <endian.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <ctype.h>
#include <libgen.h>
#include <paths.h>
#include <xfs/swab.h>

#define constpp	char * const *

#define EFSCORRUPTED	990	/* Filesystem is corrupted */

typedef off_t		xfs_off_t;
typedef __uint64_t	xfs_ino_t;
typedef __uint32_t	xfs_dev_t;
typedef __int64_t	xfs_daddr_t;
typedef char*		xfs_caddr_t;
typedef off_t		loff_t;

#ifndef	_UCHAR_T_DEFINED
typedef unsigned char	uchar_t;
#define	_UCHAR_T_DEFINED	1
#endif
typedef enum { B_FALSE,B_TRUE }	boolean_t;

#define HAVE_FID	1

static __inline__ int xfsctl(const char *path, int fd, int cmd, void *p)
{
	return ioctl(fd, cmd, p);
}

static __inline__ int platform_test_xfs_fd(int fd)
{
	struct statfs buf;
	if (fstatfs(fd, &buf) < 0)
		return 0;
	return strncmp(buf.f_fstypename, "xfs", 4) == 0;
}

static __inline__ int platform_test_xfs_path(const char *path)
{
	struct statfs buf;
	if (statfs(path, &buf) < 0)
		return 0;
	return strncmp(buf.f_fstypename, "xfs", 4) == 0;
}

static __inline__ int platform_fstatfs(int fd, struct statfs *buf)
{
	return fstatfs(fd, buf);
}

static __inline__ void platform_getoptreset(void)
{
	extern int optind;
	optind = 0;
}

static __inline__ int platform_uuid_compare(uuid_t *uu1, uuid_t *uu2)
{
	return uuid_compare(*uu1, *uu2);
}

static __inline__ void platform_uuid_unparse(uuid_t *uu, char *buffer)
{
	uuid_unparse(*uu, buffer);
}

static __inline__ int platform_uuid_parse(char *buffer, uuid_t *uu)
{
	return uuid_parse(buffer, *uu);
}

static __inline__ int platform_uuid_is_null(uuid_t *uu)
{
	return uuid_is_null(*uu);
}

static __inline__ void platform_uuid_generate(uuid_t *uu)
{
	uuid_generate(*uu);
}

static __inline__ void platform_uuid_clear(uuid_t *uu)
{
	uuid_clear(*uu);
}

static __inline__ void platform_uuid_copy(uuid_t *dst, uuid_t *src)
{
	uuid_copy(*dst, *src);
}

static __inline__ int
platform_discard_blocks(int fd, uint64_t start, uint64_t len)
{
	return 0;
}

#endif	/* __XFS_KFREEBSD_H__ */
