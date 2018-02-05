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

#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <uuid/uuid.h>
#include <libgen.h>
#include <sys/vm.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ftw.h>
#include <mach/mach_time.h>
#include <inttypes.h>
#include <stdio.h>
#include <sys/mman.h>

#include <machine/endian.h>
#define __BYTE_ORDER	BYTE_ORDER
#define __BIG_ENDIAN	BIG_ENDIAN
#define __LITTLE_ENDIAN	LITTLE_ENDIAN

#include <sys/syscall.h>
# ifndef SYS_fsctl
#  define SYS_fsctl	242
# endif

#ifndef XATTR_LIST_MAX
#define XATTR_LIST_MAX  65536
#endif

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

typedef unsigned char		__u8;
typedef signed char		__s8;
typedef unsigned short		__u16;
typedef signed short		__s16;
typedef unsigned int		__u32;
typedef signed int		__s32;
typedef unsigned long long int	__u64;
typedef signed long long int	__s64;

#define int8_t		int8_t
#define int16_t		int16_t
#define int32_t		int32_t
#define int32_t		int32_t
#define int64_t		int64_t

typedef off_t		xfs_off_t;
typedef uint64_t	xfs_ino_t;
typedef uint32_t	xfs_dev_t;
typedef int64_t		xfs_daddr_t;
typedef __u32		xfs_nlink_t;

#define fdatasync	fsync
#define memalign(a,sz)	valloc(sz)

#define O_LARGEFILE     0
#ifndef O_DIRECT
#define O_DIRECT        0
#endif
#ifndef O_SYNC
#define O_SYNC          0
#endif

#define EFSCORRUPTED	990	/* Filesystem is corrupted */
#define EFSBADCRC	991	/* Bad CRC detected */

#define HAVE_FID	1

static __inline__ int
platform_discard_blocks(int fd, uint64_t start, uint64_t len)
{
	return 0;
}

/*
 * POSIX timer replacement.
 * It really just do the minimum we need for xfs_repair.
 * Also, as setitimer can't create multiple timers,
 * the timerid things are useless - we have only one ITIMER_REAL
 * timer.
 */
#define CLOCK_REALTIME ITIMER_REAL
#define itimerspec itimerval
typedef uint64_t timer_t;
typedef double   timer_c;
typedef clock_id_t clockid_t;


static inline int timer_create (clockid_t __clock_id,
                         struct sigevent *__restrict __evp,
                         timer_t *__restrict timer)
{
	// set something, to initialize the variable, just in case
	*timer = 0;
	return 0;
}

static inline int timer_settime (timer_t timerid, int flags,
                          const struct itimerspec *__restrict timerspec,
                          struct itimerspec *__restrict ovalue)
{
	return setitimer(ITIMER_REAL, timerspec, ovalue);
}

static inline int timer_delete (timer_t timerid)
{
	struct itimerspec timespec;

	timespec.it_interval.tv_sec=0;
	timespec.it_interval.tv_usec=0;
	timespec.it_value.tv_sec=0;
	timespec.it_value.tv_usec=0;

	return setitimer(ITIMER_REAL, &timespec, NULL);
}

static inline int timer_gettime (timer_t timerid, struct itimerspec *value)
{
	return getitimer(ITIMER_REAL, value);
}

/* FSR */

#  include <sys/mount.h>
#  include <sys/param.h>
#include <sys/ucred.h>
#include <errno.h>
#define		_PATH_MOUNTED   "/etc/mtab"

struct mntent
{
	char *mnt_fsname;
	char *mnt_dir;
	char *mnt_type;
	char *mnt_opts;
	int mnt_freq;
	int mnt_passno;
};

static inline void mntinfo2mntent (struct statfs * stats, struct mntent * mnt) {
	mnt->mnt_fsname = stats->f_mntfromname;
	mnt->mnt_dir = stats->f_mntonname;
	mnt->mnt_type = stats->f_fstypename;
}



/**
 * Abstraction of mountpoints.
 */
struct mntent_cursor {
	FILE *mtabp;
	struct statfs *stats;
	int count;
	int i;
};

/**
 * OS X uses getmntinfo, which doesn't use a mtab file. So we just ignore it.
 */
static inline int platform_mntent_open(struct mntent_cursor * cursor, char *mtab)
{
	if ((cursor->count = getmntinfo(&cursor->stats, 0)) < 0) {
		fprintf(stderr, "Error: getmntinfo() failed: %s\n", strerror(errno));
		return 1;
	}
	cursor->i = 0;
	return 0;
}

static inline struct mntent * platform_mntent_next(struct mntent_cursor * cursor)
{
	struct mntent * t = NULL;
	if (cursor->i >= cursor->count){
		return NULL;
	}
	mntinfo2mntent(&cursor->stats[cursor->i], t);
	cursor->i++;
	return t;
}

static inline void platform_mntent_close(struct mntent_cursor * cursor)
{
	cursor->count = 0;
	cursor->i = 0;
}

/* check whether we have to define FS_IOC_FS[GS]ETXATTR ourselves */
#ifndef HAVE_FSXATTR
struct fsxattr {
	__u32		fsx_xflags;	/* xflags field value (get/set) */
	__u32		fsx_extsize;	/* extsize field value (get/set)*/
	__u32		fsx_nextents;	/* nextents field value (get)	*/
	__u32		fsx_projid;	/* project identifier (get/set) */
	__u32		fsx_cowextsize;	/* cow extsize field value (get/set) */
	unsigned char	fsx_pad[8];
};

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

#endif	/* __XFS_DARWIN_H__ */
