// SPDX-License-Identifier: LGPL-2.1
/*
 * Copyright (c) 2004-2005 Silicon Graphics, Inc.  All Rights Reserved.
 */
#ifndef __XFS_LINUX_H__
#define __XFS_LINUX_H__

/*
 * Reminder: anything added to this file will be compiled into downstream
 * userspace projects!
 */

#include <uuid.h>
#include <sys/vfs.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <malloc.h>
#include <getopt.h>
#include <errno.h>
#include <endian.h>
#include <stdbool.h>
#include <stdio.h>
#include <signal.h>
#include <asm/types.h>
#include <mntent.h>
#include <fcntl.h>
#if defined(HAVE_FALLOCATE)
#include <linux/falloc.h>
#endif
#ifdef OVERRIDE_SYSTEM_FSXATTR
# define fsxattr sys_fsxattr
#endif
#include <linux/fs.h> /* fsxattr defintion for new kernels */
#ifdef OVERRIDE_SYSTEM_FSXATTR
# undef fsxattr
#endif
#include <unistd.h>
#include <assert.h>

static __inline__ int xfsctl(const char *path, int fd, int cmd, void *p)
{
	return ioctl(fd, cmd, p);
}

/*
 * platform_test_xfs_*() implies that xfsctl will succeed on the file;
 * on Linux, at least, special files don't get xfs file ops,
 * so return 0 for those
 */

static __inline__ int platform_test_xfs_fd(int fd)
{
	struct statfs statfsbuf;
	struct stat statbuf;

	if (fstatfs(fd, &statfsbuf) < 0)
		return 0;
	if (fstat(fd, &statbuf) < 0)
		return 0;
	if (!S_ISREG(statbuf.st_mode) && !S_ISDIR(statbuf.st_mode))
		return 0;
	return (statfsbuf.f_type == 0x58465342);	/* XFSB */
}

static __inline__ int platform_test_xfs_path(const char *path)
{
	struct statfs statfsbuf;
	struct stat statbuf;

	if (statfs(path, &statfsbuf) < 0)
		return 0;
	if (stat(path, &statbuf) < 0)
		return 0;
	if (!S_ISREG(statbuf.st_mode) && !S_ISDIR(statbuf.st_mode))
		return 0;
	return (statfsbuf.f_type == 0x58465342);	/* XFSB */
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

static __inline__ int platform_uuid_parse(const char *buffer, uuid_t *uu)
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

#ifndef BLKDISCARD
#define BLKDISCARD	_IO(0x12,119)
#endif

static __inline__ int
platform_discard_blocks(int fd, uint64_t start, uint64_t len)
{
	uint64_t range[2] = { start, len };

	if (ioctl(fd, BLKDISCARD, &range) < 0)
		return errno;
	return 0;
}

#define ENOATTR		ENODATA	/* Attribute not found */
#define EFSCORRUPTED	EUCLEAN	/* Filesystem is corrupted */
#define EFSBADCRC	EBADMSG	/* Bad CRC detected */

typedef off_t		xfs_off_t;
typedef uint64_t	xfs_ino_t;
typedef uint32_t	xfs_dev_t;
typedef int64_t		xfs_daddr_t;
typedef __u32		xfs_nlink_t;

/**
 * Abstraction of mountpoints.
 */
struct mntent_cursor {
	FILE *mtabp;
};

static inline int platform_mntent_open(struct mntent_cursor * cursor, char *mtab)
{
	cursor->mtabp = setmntent(mtab, "r");
	if (!cursor->mtabp) {
		fprintf(stderr, "Error: cannot read %s\n", mtab);
		return 1;
	}
	return 0;
}

static inline struct mntent * platform_mntent_next(struct mntent_cursor * cursor)
{
	return getmntent(cursor->mtabp);
}

static inline void platform_mntent_close(struct mntent_cursor * cursor)
{
	endmntent(cursor->mtabp);
}

#if defined(FALLOC_FL_ZERO_RANGE)
static inline int
platform_zero_range(
	int		fd,
	xfs_off_t	start,
	size_t		len)
{
	int ret;

	ret = fallocate(fd, FALLOC_FL_ZERO_RANGE, start, len);
	if (!ret)
		return 0;
	return -errno;
}
#else
#define platform_zero_range(fd, s, l)	(-EOPNOTSUPP)
#endif

/*
 * Use SIGKILL to simulate an immediate program crash, without a chance to run
 * atexit handlers.
 */
static inline void
platform_crash(void)
{
	kill(getpid(), SIGKILL);
	assert(0);
}

/*
 * Check whether we have to define FS_IOC_FS[GS]ETXATTR ourselves. These
 * are a copy of the definitions moved to linux/uapi/fs.h in the 4.5 kernel,
 * so this is purely for supporting builds against old kernel headers.
 */
#if !defined FS_IOC_FSGETXATTR || defined OVERRIDE_SYSTEM_FSXATTR
struct fsxattr {
	__u32		fsx_xflags;	/* xflags field value (get/set) */
	__u32		fsx_extsize;	/* extsize field value (get/set)*/
	__u32		fsx_nextents;	/* nextents field value (get)	*/
	__u32		fsx_projid;	/* project identifier (get/set) */
	__u32		fsx_cowextsize;	/* cow extsize field value (get/set) */
	unsigned char	fsx_pad[8];
};
#endif

#ifndef FS_IOC_FSGETXATTR
/*
 * Flags for the fsx_xflags field
 */
#define FS_XFLAG_REALTIME	0x00000001	/* data in realtime volume */
#define FS_XFLAG_PREALLOC	0x00000002	/* preallocated file extents */
#define FS_XFLAG_IMMUTABLE	0x00000008	/* file cannot be modified */
#define FS_XFLAG_APPEND		0x00000010	/* all writes append */
#define FS_XFLAG_SYNC		0x00000020	/* all writes synchronous */
#define FS_XFLAG_NOATIME	0x00000040	/* do not update access time */
#define FS_XFLAG_NODUMP		0x00000080	/* do not include in backups */
#define FS_XFLAG_RTINHERIT	0x00000100	/* create with rt bit set */
#define FS_XFLAG_PROJINHERIT	0x00000200	/* create with parents projid */
#define FS_XFLAG_NOSYMLINKS	0x00000400	/* disallow symlink creation */
#define FS_XFLAG_EXTSIZE	0x00000800	/* extent size allocator hint */
#define FS_XFLAG_EXTSZINHERIT	0x00001000	/* inherit inode extent size */
#define FS_XFLAG_NODEFRAG	0x00002000	/* do not defragment */
#define FS_XFLAG_FILESTREAM	0x00004000	/* use filestream allocator */
#define FS_XFLAG_DAX		0x00008000	/* use DAX for IO */
#define FS_XFLAG_HASATTR	0x80000000	/* no DIFLAG for this	*/

#define FS_IOC_FSGETXATTR     _IOR ('X', 31, struct fsxattr)
#define FS_IOC_FSSETXATTR     _IOW ('X', 32, struct fsxattr)

#endif

#ifndef FS_XFLAG_COWEXTSIZE
#define FS_XFLAG_COWEXTSIZE	0x00010000	/* CoW extent size allocator hint */
#endif

/*
 * Reminder: anything added to this file will be compiled into downstream
 * userspace projects!
 */

#endif	/* __XFS_LINUX_H__ */
