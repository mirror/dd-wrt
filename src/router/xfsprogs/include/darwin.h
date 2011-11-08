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
#ifndef __XFS_DARWIN_H__
#define __XFS_DARWIN_H__

#include <uuid/uuid.h>
#include <libgen.h>
#include <sys/vm.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

#include <machine/endian.h>
#define __BYTE_ORDER	BYTE_ORDER
#define __BIG_ENDIAN	BIG_ENDIAN
#define __LITTLE_ENDIAN	LITTLE_ENDIAN

#include <sys/syscall.h>
# ifndef SYS_fsctl
#  define SYS_fsctl	242
# endif
static __inline__ int xfsctl(const char *path, int fd, int cmd, void *p)
{
	return syscall(SYS_fsctl, path, cmd, p, 0);
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
	extern int optreset;
	optreset = 0;
}

static __inline__ int platform_uuid_compare(uuid_t *uu1, uuid_t *uu2)
{
	return uuid_compare(uu1, uu2, NULL);
}

static __inline__ void platform_uuid_unparse(uuid_t *uu, char *buffer)
{
	uint32_t status;
	char *s;
	uuid_to_string(uu, &s, &status);
	if (status == uuid_s_ok)
		strcpy(buffer, s);
	else buffer[0] = '\0';
	free(s);
}

static __inline__ int platform_uuid_parse(char *buffer, uuid_t *uu)
{
	uint32_t status;
	uuid_from_string(buffer, uu, &status);
	return (status == uuid_s_ok);
}

static __inline__ int platform_uuid_is_null(uuid_t *uu)
{
	return uuid_is_nil(uu, NULL);
}

static __inline__ void platform_uuid_generate(uuid_t *uu)
{
	uuid_create(uu, NULL);
}

static __inline__ void platform_uuid_clear(uuid_t *uu)
{
	uuid_create_nil(uu, NULL);
}

static __inline__ void platform_uuid_copy(uuid_t *dst, uuid_t *src)
{
	memcpy(dst, src, sizeof(uuid_t));
}

#define __int8_t	int8_t
#define __int16_t	int16_t
#define __int32_t	int32_t
#define __int32_t	int32_t
#define __int64_t	int64_t
#define __uint8_t	u_int8_t
#define __uint16_t	u_int16_t
#define __uint32_t	u_int32_t
#define __uint64_t	u_int64_t
#define loff_t		off_t
#define off64_t		off_t

typedef off_t		xfs_off_t;
typedef u_int64_t	xfs_ino_t;
typedef u_int32_t	xfs_dev_t;
typedef int64_t		xfs_daddr_t;
typedef char*		xfs_caddr_t;

typedef unsigned char	uchar_t;
#define stat64		stat
#define fstat64		fstat
#define lseek64		lseek
#define pread64		pread
#define pwrite64	pwrite
#define ftruncate64	ftruncate
#define fdatasync	fsync
#define memalign(a,sz)	valloc(sz)

#define O_LARGEFILE     0
#ifndef O_DIRECT
#define O_DIRECT        0
#endif
#ifndef O_SYNC
#define O_SYNC          0
#endif

#define B_FALSE		0
#define B_TRUE		1

#define ENOATTR		989     /* Attribute not found */
#define EFSCORRUPTED	990	/* Filesystem is corrupted */
#define constpp		char * const *

#define HAVE_FID	1

static __inline__ int
platform_discard_blocks(int fd, uint64_t start, uint64_t len)
{
	return 0;
}

#endif	/* __XFS_DARWIN_H__ */
